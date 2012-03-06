#include <fstream>
#include<pthread.h>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <string.h>
#include "Record.h"
#include "Schema.h"
#include "SortedFile.h"
#include "Defs.h"
#include "CommonClass.h"
#include "HeapFile.h"
#include "File.h"
#include "BigQ.h"

using namespace std;
//extern char * catalog_path;
//extern char * nation;
//Schema *rschema =new Schema (catalog_path, nation);

SortedFile::SortedFile() {
    fileName =NULL;
    mode = READ;
    pageNumber =-1;
    getNextCNF =false;
    queryMaker=false;
}

SortedFile::~SortedFile(){
}

bool SortedFile::checkISDirty() {

    Record bigQRecord;
    Page filePage;
    Page filePage1;
    int fileLength;
    HeapFile newFile;
    ComparisonEngine compeng;
    char * mergeFileName;
    if (mode == WRITE) {

        mode = READ;
        //generate a random number for temp file that will be used for merging
        time_t seconds;
        time(&seconds);
        srand((unsigned int) seconds);

        mergeFileName = new char[strlen(fileName) + 6];
        char random[6];

        sprintf(random, "%d", rand()%10000);

        strcpy(mergeFileName, fileName);
        strcat(mergeFileName, random);
        newFile.Create( mergeFileName, heap, NULL);
        input->ShutDown();
        output->Remove(&bigQRecord);
        pageNumber = -1;
        fileLength = fp.GetLength();
        
        if (fileLength > 0) {
            while (1) {
                fp.GetPage(&filePage, ++pageNumber);
                
                //check if we can fetch one more page
                if (pageNumber < fileLength - 2) {
                    fp.GetPage(&filePage1, pageNumber + 1);
                } else
                    break;

                Record secondPageFirstRecord;
                int response = filePage1.GetFirst(&secondPageFirstRecord);
                /*compare secondPageFirstRecord with the first record of BigQ
                 * if greater than secondPageFirstRecord then insert first page into HeapFile
                 * and continue
                 */
                if (response != 0 && compeng.Compare(&bigQRecord, &secondPageFirstRecord,headerInformation.getSortOrder()) >= 0) {
                    newFile.Add(filePage);
                    filePage.EmptyItOut();
                    filePage1.EmptyItOut();
                } else {
                    //continue with second phase of merge by adding one record at a time
                    filePage1.EmptyItOut();
                    break;
                }
            }
            Record fileCurrentRecord;
            //filePage is already fetched
            int response = filePage.GetFirst(&fileCurrentRecord);
            bool bigQDone = false;
            while (1) {
                //compare filePage and bigQRecord and insert whichever is less into the newPage
                if (response != 0 && compeng.Compare(&bigQRecord, &fileCurrentRecord, headerInformation.getSortOrder()) >= 0) {
                    /*
                     * If record is inserted from File instance then get the next Record
                     * If there are no more records in the page. Fetch the next page.
                     * If there are no more pages in the file. then copy all the recrods from BigQ directly
                     */
                    newFile.Add(fileCurrentRecord);
                    //fetch the next record
                    if (filePage.GetFirst(&fileCurrentRecord) == 0) {
                        //fetch the next page
                        if (pageNumber < fp.GetLength() - 2) {
                            //cout<<"Fetch the next page\n";
                            //getchar();
                            filePage.EmptyItOut();
                            fp.GetPage(&filePage, ++pageNumber);
                            response = filePage.GetFirst(&fileCurrentRecord);
                            continue;
                        } else {
                            break;
                        }
                    } else{
                        response = 1;
                    }
                } else {
                    newFile.Add(bigQRecord);
                    //fetch the next record from the BigQ output queue
                    if (output->Remove(&bigQRecord) == 0) {
                        bigQDone = true;
                        break;
                    }
                }
            }
            if (bigQDone == false) {
                // it means there are still records left in the bigQ
                newFile.Add(bigQRecord);
                while (output->Remove(&bigQRecord)) {
                    newFile.Add(bigQRecord);
                }
            }else{
                // there are still unmerged records left in the file
                while (1) {
                    newFile.Add(fileCurrentRecord);
                    //fetch the next record
                    if (filePage.GetFirst(&fileCurrentRecord) == 0) {
                        //fetch the next page
                        if (pageNumber < fp.GetLength() - 2) {
                            filePage.EmptyItOut();
                            fp.GetPage(&filePage, ++pageNumber);
                            response = filePage.GetFirst(&fileCurrentRecord);
                            continue;
                        } else {
                            break;
                        }
                    }
                }
            }
        }else
        {
            //file length is zero, simply insert the record
            newFile.Add(bigQRecord);
            while (output->Remove(&bigQRecord)) {
                newFile.Add(bigQRecord);
            }
        }
        newFile.Close();
        fp.Close();
        
        char *tempHeader = new char[strlen(mergeFileName) + HEADERFILE_LENGTH+1];
        strcpy(tempHeader,mergeFileName);
        strcat(tempHeader,HEADERFILE_EXT);
        
        //remove the temp file header
        remove(tempHeader);
        rename(mergeFileName,fileName);
        delete []tempHeader;
        delete []mergeFileName;
        delete input;
        delete output;
        delete bigQ;
        fp.Open(1, fileName);
    }else
        return 0;
    return 1;
}

int SortedFile::Create(char *name, fType f_type, void *startup) {

    header *start = (header *) startup;
    //create a new file
    int notNew = 0;
    //open the file in File class, since notNew is 0 it will create a new file
    fp.Open(notNew, name);

    fileName = strdup(name);
    
    //create a header file
    char *headerFileName = new char[strlen(name) + HEADERFILE_LENGTH + 1];
    strcpy(headerFileName, name);
    strcat(headerFileName, HEADERFILE_EXT);

    fstream fobj(headerFileName, fstream::out);
    HeaderInformation headerInfo;
    headerInfo.setFileType(f_type);
    headerInfo.setRunLength(start->l);
    headerInfo.setSortOrder(*(start->o));
        
    fobj.write((char *) &headerInfo, sizeof(headerInfo));
    fobj.close();
    delete []headerFileName;
    return 0;
}

void SortedFile::Load(Schema &f_schema, char *loadpath) {

    if(getNextCNF ==true)
        getNextCNF=false;
    
    FILE *tblfile = fopen(loadpath, "r");
    Record temp;

    while (temp.SuckNextRecord(&f_schema, tblfile)) {
        Add(temp);
    }
}

int SortedFile::Open(char *name) {
    //open the file in File class
    int notNew = 1;

    fp.Open(notNew, name);
    fileName = strdup(name);

    char *headerFileName = new char[strlen(name) + HEADERFILE_LENGTH + 1];
    strcpy(headerFileName, name);
    strcat(headerFileName, HEADERFILE_EXT);

    fstream fobj(headerFileName, fstream::in);

    fobj.read((char *) & headerInformation, sizeof (HeaderInformation));
    delete []headerFileName;
    return 0;
}

void SortedFile::MoveFirst() {

    if(getNextCNF ==true)
        getNextCNF=false;

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

int SortedFile::Close() {
    
    //check if the page isDirty then merge the differential file and current file
    if(getNextCNF ==true)
        getNextCNF=false;
    
    checkISDirty();
    fp.Close();

    if(fileName !=NULL){
        free(fileName);
    }
}

int SortedFile::GetLength() {
    return (fp.GetLength());
}

void SortedFile::GetPage(Page &putItHere, off_t whichPage) {
    if(getNextCNF ==true)
        getNextCNF=false;

    checkISDirty();
    fp.GetPage(&putItHere, whichPage);
}

void SortedFile::Add(Page &addMe) {

    if(getNextCNF ==true)
        getNextCNF=false;
    
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
    } else {
        fp.AddPage(&addMe, fileLength - 1);
    }
}

void SortedFile::Add(Record &rec) {

    if(getNextCNF ==true)
        getNextCNF=false;
    // check the dbFile instance mode
    if (mode == READ) {
        // initialize the BigQ structure
        
        input = new Pipe(PIPE_SIZE);
        output = new Pipe(PIPE_SIZE);
        bigQ = new BigQ(*input, *output, *headerInformation.getSortOrder(), headerInformation.getRunLength());
        mode = WRITE;
    }
    input->Insert(&rec);
}

int SortedFile::GetNext(Record &fetchMe) {
    if(getNextCNF ==true)
        getNextCNF=false;

    /* if current Page is dirty write the page back to disk and move the current
     * page to first page in the file
     */
    if (checkISDirty()) {
        MoveFirst();
    }
    if(fp.GetLength() ==0)
        return 0;
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
}
int SortedFile::GetNext(Record &fetchMe, CNF &applyMe, Record &literal) {

    ComparisonEngine compeng;

    if (checkISDirty()) {
        MoveFirst();
    }
    if(fp.GetLength() ==0)
        return 0;
    
    if(getNextCNF ==false){
        getNextCNF=true;
        if(applyMe.QueryMaker(*(headerInformation.getSortOrder()), query)==1){
            queryMaker=true;
            if(BinarySearch(query, literal)==0){
                return 0;    
            }
        }
        else
            queryMaker = false;
    }
    
    while (1) {
        if (currentPage.GetFirst(&fetchMe)) {
            
            if (queryMaker ==false || compeng.Compare(&fetchMe, headerInformation.getSortOrder(), &literal, &query) <= 0) {
                if(compeng.Compare(&fetchMe,&literal,&applyMe))
                    return 1;
                else{
                    continue;
                }
            }
            else{
                cout<<"record failed the query check"<<endl;
                return 0;
            }
        }else{
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
}

int SortedFile::BinarySearch (OrderMaker &query, Record &literal)
{
    int lowPageNumber = pageNumber;
    int highPageNumber = fp.GetLength() -2;
    int midPageNumber;
    Page midPage;
    Record temp;
    ComparisonEngine compeng;
    
    bool linearScanPage =false;
    while (lowPageNumber <= highPageNumber){
        
        midPageNumber = ((highPageNumber + lowPageNumber) / 2);

        //cout<<"MidPageNumber"<<midPageNumber<<" Low page number"<<lowPageNumber<<" High Page number"<<highPageNumber<<endl;
        // getting the middle page
        fp.GetPage(&midPage,midPageNumber);
        midPage.GetFirst(&temp);

        int res = compeng.Compare(&temp,headerInformation.getSortOrder(),&literal,&query);
        if ( res<0){
            //cout<<"Less than\n";
            if(highPageNumber ==lowPageNumber){
                linearScanPage =true;
                break;
            }
            //check that we don't skip the records in current midPageNumber
            fp.GetPage(&midPage,midPageNumber+1);
            midPage.GetFirst(&temp);

            res = compeng.Compare(&temp,headerInformation.getSortOrder(),&literal,&query);

            if (res < 0) {
                lowPageNumber = midPageNumber + 1; // repeat search in top half.
            }
            else {
                linearScanPage =true;
                break; //record range start from some record in pageNumberMidPage
            }
        }
        else if (res>0) {
            highPageNumber = midPageNumber - 1; // repeat search in bottom half.
        }
        else{
            linearScanPage = true;
            if(highPageNumber ==lowPageNumber)
                break;
            else
                highPageNumber = midPageNumber;// found it.
        }
    }

    if(linearScanPage ==false)
        return 0;

    // if page number is not equal to currentPage number then fetch tne new page
    if(midPageNumber != pageNumber){
        fp.GetPage(&currentPage,midPageNumber);
        pageNumber = midPageNumber;
    }
    // otherwise search linearly on currentPage
    return 1;
}
