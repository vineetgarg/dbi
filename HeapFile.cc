#include "HeapFile.h"
#include "File.h"
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include "Defs.h"

/* Constructor for HeapFile
 * Initialize all the instance variable
 */
HeapFile::HeapFile() {

    pageNumber = -1;
    pendingLastPage = 0;
    isDirty = false;
}

/* Destructor for HeapFile
 * Close all the external files or memory opened
 */
HeapFile::~HeapFile() {
    /*no need to check for file close or anything
     * as there is none in other classes too, so I assume that close() function
     * will be called before HeapFile runs out of scope
     */
}

/* Open: Open a existing binary file for reading and writing
 * Open the file that already exist
 * Failure, File does not exit or unable to open the file: Terminate the process
 * Sucess : File opened successfully Returns 1
 */
int HeapFile::Open(char *name) {

    //open the file in File class
    int notNew = 1;

    fp.Open(notNew, name);
    //cout << "Heap:File opened successfully:"<<name<<endl;

    /* add code to open header file, not required in this project
     * as only one kind of file exist and so we don't require metadata
     */
    return 1;
}

/* Close() Function:: Close the file that we have opened
 * If isDirty flag is true then write the pending write to file
 * and then close the file
 * NOTE: It does not check whether file is opened or not
 * so operation should be only executed on the opened file
 */
int HeapFile::Close() {

    //write the current page to memory if isDirty is true
    checkISDirty();

    //close the file
    if (fp.Close()) {
        pageNumber = -1;
        //cout << "File closed successfuly\n";
        return 1;
    }
    /*
    else {
        cout << "Error occured while closing the file\n";
        return 0;
    }
     */
}

/* Create(): Create a new binary file and corrsponding header file
 * for reading and writing and store metadata in the header file
 * Failure, Unable to create the file: Exit the process
 * Success : File created successfully Returns 1
 */
int HeapFile::Create(char *name, fType myType, void *startup) {

    //create a new file
    int notNew = 0;
    //open the file in File class, since notNew is 0 it will create a new file
    fp.Open(notNew, name);

    //create a header file now
    char *headerFileName = (char *) malloc(strlen(name) + HEADERFILE_LENGTH + 1);
    strcpy(headerFileName, name);
    strcat(headerFileName, HEADERFILE_EXT);

    fstream fobj(headerFileName, fstream::out);
    HeaderInformation headerInfo;
    headerInfo.setFileType(myType);

//    cout << "Order Maker in create is as follows:" << endl;
//    headerInfo.getSortOrder()->Print();

    fobj.write((char *) &headerInfo, sizeof(headerInfo));
    fobj.close();

    //cout << "File created successfully " << name << endl;
    //cout << "Header file created successfully " << headerFileName << endl;
    free(headerFileName);
    return 1;
}

/* Load: Open the text file for a table, read the records and store data in a
 * already opened binary file
 */
void HeapFile::Load(Schema &mySchema, char *loadMe) {

    //open the file for reading
    FILE *tableFile = fopen(loadMe, "r");
    int recordCounter = 0;
    if (tableFile == NULL) {
        cout << "Unable to open the File " << loadMe << endl;
        cout << "File does not exist or you don't have read permission on that file\n";
        exit(1);
    }
    //now start reading the file
    Record currentRecord; //since record is consumed so no memory leak
    while (currentRecord.SuckNextRecord(&mySchema, tableFile)) {
        Add(currentRecord);
        recordCounter++;
    }
    // to write last page back to disk
    checkISDirty();
    cout << recordCounter << " records has loaded into file " << endl;
    fclose(tableFile);
}

bool HeapFile::checkISDirty() {
    if (isDirty) {
        off_t fileLength = fp.GetLength();
        if (fileLength == 0) {
            fp.AddPage(&currentPage, fileLength);
        } else {
            fp.AddPage(&currentPage, fileLength - 1 - pendingLastPage);
        }
        //Release the memory of currentPage
        currentPage.EmptyItOut();
        pendingLastPage = 0;
        isDirty = false;
        return 1;
    }
    return 0;
}

/* Use the current page and return the first record
 * otherwise read the next page and retrieve first record from that.
 * If isDirty flag is TRUE then write the records back to disk
 * Return 1 on success and 0 on failure
 */
int HeapFile::GetNext(Record &fetchMe) {

    /* if current Page is dirty write the page back to disk and move the current
     * page to first page in the file
     */
    if (checkISDirty()) {
        MoveFirst();
    }
    while (1) {
        if (currentPage.GetFirst(&fetchMe)) {
            return 1;
        } else {
            /*
             * fetch the next page and if this is the end of page
             * then returns 0
             */
            /* Length -2 as there is no record in the first page and getPage,
             * do which++ before retrieving the page
             */
            if (pageNumber < fp.GetLength() - 2) {
                //do clean up, we might not need this, as we have read all the records
                currentPage.EmptyItOut();
                fp.GetPage(&currentPage, ++pageNumber);
                continue;
            } else
                return 0;
        }
    }
    // code will never reach this point
    return 1;
}

/*Use the current page and return the first record that satisfy the CNF criteria
 * otherwise read the next page and retrieve first record from that.
 * If isDirty flag is TRUE then write the records back to disk
 * Return 1 on success and 0 on failure
 */
int HeapFile::GetNext(Record &fetchMe, CNF &applyMe, Record &literal) {

    ComparisonEngine comp;
    /* if current Page is dirty write the page back to disk and move the current
     * page to first page in the file
     */
    if (checkISDirty()) {
        MoveFirst();
    }
    while (1) {
        if (currentPage.GetFirst(&fetchMe)) {
            if (comp.Compare(&fetchMe, &literal, &applyMe))
                return 1;
            else
                continue;
        } else {
            /*
             * fetch the next page and if this is the end of page
             * then returns NULL
             * length -2 as there is no record in the first page
             */
            if (pageNumber < fp.GetLength() - 2) {
                // do clean up, we might not need this, as we have already read all the records
                currentPage.EmptyItOut();
                fp.GetPage(&currentPage, ++pageNumber);
                continue;
            } else
                return 0;
        }
    }
    return 1;
}

/*
 * It sets the page number to 0 and retrieve the first page
 */
void HeapFile::MoveFirst() {

    //write the page back to memory and free up the memory
    checkISDirty();
    //dump the current page and load first page again
    currentPage.EmptyItOut();
    /*
     * get the first page as we don't know where page will be
     * check for fileLength otherwise call to GetPage will terminate the process
     * by doing exit() from GetPage in case of any error
     */
    pageNumber = 0;
    if (fp.GetLength() > 0)
        fp.GetPage(&currentPage, pageNumber);
    else
        cerr << "File length is zero. So cannot set the page to first records\n";
}

void HeapFile::Add(Record &addMe) {

    /*if file length is not zero and isDirty is false, then retrieve the last page
     * otherwise if isDirty is true, then current page is already equals to last page
     * or if fileLength is zero then add records to current page and then write records
     * back to file.
     */
    if (isDirty == false) {
        //retrieve the file length
        off_t fileLength;
        fileLength = fp.GetLength();
        if (fileLength != 0) {
            //emptied out the current page, as we will be using the same page
            currentPage.EmptyItOut();
            //now get the last page
            fp.GetPage(&currentPage, fileLength - 2);
            pendingLastPage = 1;
        }
    }
    //now add the record at the end of the last page
    if (currentPage.Append(&addMe) == 0) {
        //retrieve the file length
        off_t fileLength = fp.GetLength();
        if (fileLength == 0) {
            fp.AddPage(&currentPage, 0);
        } else {
//            cout << "Page become full\n";
//            getchar();
            fp.AddPage(&currentPage, fileLength - 1 - pendingLastPage);
            pendingLastPage = 0;
        }
        currentPage.EmptyItOut();
        //now add new record to the page that we just made empty
        currentPage.Append(&addMe);
    }
    //cout << "Record appended\n";
    isDirty = true;
}

int HeapFile::GetLength(){
    return(fp.GetLength());
}
void HeapFile::Add(Page &addMe) {

    /*if file length is not zero and isDirty is false, then retrieve the last page
     * otherwise if isDirty is true, then current page is already equals to last page
     * or if fileLength is zero then add records to current page and then write records
     * back to file.
     */
    checkISDirty();

    off_t fileLength;
    fileLength = fp.GetLength();
    if (fileLength == 0) {
        fp.AddPage(&addMe, 0);
    }else{
        fp.AddPage(&addMe, fileLength - 1);
    }
}

void HeapFile::GetPage (Page &putItHere, off_t whichPage)
{
    //cout<<"Page retrieved from heap\n";
    fp.GetPage(&putItHere,whichPage);
}
