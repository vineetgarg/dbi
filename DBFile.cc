/* File - DBFile.cc
* @Author - Vineet Garg
*/
#include "DBFile.h"
#include "File.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "Defs.h"
#include "CommonClass.h"
#include <fstream>

/* Constructor for DBFile
 * Initialize all the instance variable
 */
DBFile::DBFile() {
    databaseFile = NULL;
}

/* Destructor for DBFile
 * Close all the external files or memory opened
 */
DBFile::~DBFile() {
    /*no need to check for file close or anything
     * as there is none in other classes too, so I assume that close() function
     * will be called before dbFile runs out of scope
     */
    if(databaseFile !=NULL){
        delete databaseFile;
        databaseFile = NULL;
    }
}

/* Open: Open a existing binary file for reading and writing
 * Open the file that already exist
 * Failure, File does not exit or unable to open the file: Terminate the process
 * Sucess : File opened successfully Returns 1
 */
int DBFile::Open(char *name) {

    fType myType;
    if (databaseFile != NULL) {
        cout << "Error:File is already open\n";
        return 0;
    } else {
        char *headerFileName = (char *) malloc(strlen(name) + HEADERFILE_LENGTH + 1);
        strcpy(headerFileName, name);
        strcat(headerFileName, HEADERFILE_EXT);

        fstream fobj(headerFileName, fstream::in);
        HeaderInformation temp;

        fobj.read((char *) & temp, sizeof (HeaderInformation));
        myType = temp.getFileType();
        free(headerFileName);
    }
    if (myType == 0) {
        HeapFile *temp = new HeapFile();
        databaseFile = (GenericDBFile *) temp;
        return(databaseFile->Open(name));
    }
    if (myType == 1) {
        SortedFile *temp = new SortedFile();
        databaseFile = (GenericDBFile *) temp;
        return(databaseFile->Open(name));
    }
    return 0;
}

int DBFile::Close() {

    if(databaseFile !=NULL){
        int res = databaseFile->Close();
        delete databaseFile;
        databaseFile = NULL;
        return res;
    }
    return 1;
}

/* Create(): Create a new binary file and corrsponding header file
 * for reading and writing and store metadata in the header file
 * Failure, Unable to create the file: Exit the process
 * Success : File created successfully Returns 1
 */
int DBFile::Create(char *name, fType myType, void *startup) {

    if (myType == 0) {
        HeapFile *temp = new HeapFile();
        databaseFile = (GenericDBFile *) temp;
        return(databaseFile->Create(name, myType, startup));
    }
    if (myType == 1) {
        SortedFile *temp = new SortedFile();
        databaseFile = (GenericDBFile *) temp;
        return(databaseFile->Create(name, myType, startup));
    }
}

/* Load: Open the text file for a table, read the records and store data in a
 * already opened binary file
 */
void DBFile::Load(Schema &mySchema, char *loadMe) {

    /* 1. If database file is NULL gives the error message file is not open
     * 2. else load the database file
     */
    if (databaseFile == NULL) {
        cout << "Error:File is not open. Load failed\n";
    } else
        databaseFile->Load(mySchema, loadMe);
}

/* Use the current page and return the first record
 * otherwise read the next page and retrieve first record from that.
 * If isDirty flag is TRUE then write the records back to disk
 * Return 1 on success and 0 on failure
 */
int DBFile::GetNext(Record &fetchMe) {

    // if databassefile NULL then the file not yet opened or else call databasefile.GetNext
    if (databaseFile == NULL) {
        cout << "Error:File is not open. GetNext failed\n";
        return 0;
    } else
        return(databaseFile->GetNext(fetchMe));   
}

/*Use the current page and return the first record that satisfy the CNF criteria
 * otherwise read the next page and retrieve first record from that.
 * If isDirty flag is TRUE then write the records back to disk
 * Return 1 on success and 0 on failure
 */
int DBFile::GetNext(Record &fetchMe, CNF &applyMe, Record &literal) {

    // if databassefile NULL then the file not yet opened or else call databasefile.GetNext
    if (databaseFile == NULL) {
        cout << "Error:File is not open. GetNext failed\n";
        return 0;
    } else
        return(databaseFile->GetNext(fetchMe, applyMe, literal));
}

/*
 * It sets the page number to 0 and retrieve the first page
 */
void DBFile::MoveFirst() {
    // if databassefile NULL then the file not yet opened or else call databasefile.MoveFirst()
    if (databaseFile == NULL) {
        cout << "Error:File is not open. MoveFirst() failed\n";
    } else
        databaseFile->MoveFirst();
}

void DBFile::Add(Record &addMe) {
    // if databassefile NULL then the file not yet opened or else call databasefile.addRECORD
    if (databaseFile == NULL) {
        cout << "Error:File is not open. Add(record) failed\n";
    } else
        databaseFile->Add(addMe);
}
