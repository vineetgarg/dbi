/*
 * File:   SortedFile.h
 */

#ifndef _SortedFile_H
#define	_SortedFile_H

#include "CommonClass.h"
#include "GenericDBFile.h"
#include "Defs.h"
#include "File.h"
#include "Pipe.h"

using namespace std;
class BigQ;

class SortedFile: virtual public GenericDBFile {
    
    char *fileName;
    // fp points to binary file
    File fp;
    //Current page object to perform subsequent getNext() and add() operations
    Page currentPage;
    // page number to retrieve next page when you run out of page while doing getNext()
    int pageNumber;

    //instance variable of BigQ
    BigQ *bigQ;
    // Instance variable to store header information correspond to a file
    HeaderInformation headerInformation;

    Pipe *input;
    Pipe *output;
    fileMode mode;
    bool getNextCNF;
    OrderMaker query;
    bool queryMaker;

    /* This is the binary search used in sorted file to get records.
     * It returns 0 in case record not accepted by query OR
     * runs off the end of the file
     * returns 1 in case we found the page on which record can be there
    */
    int BinarySearch (OrderMaker &query, Record &literal);
    
    /*private function to check if ISDirty flag is true then
     * merge the differential file and current file into current File
     */
    bool checkISDirty();
    
public:
    /* Constructor for SortedFile
    * Initialize all the instance variable
    */
    SortedFile ();

    /* Destructor for SortedFile
    * Close all the external files or memory opened
    */
    ~SortedFile ();

    /*
     * Set the current page to first page. It checks if ISDirty flag is true
     * then write the records back to disk
     */
    void MoveFirst();

    /*
     * Add the records to binary file.
     * Fetch the last page and insert at the end. Use ISDirty flag to do
     * delayed write, so that we don't have to read and write the page back to
     * disk for every Add function call
     */

    void Add(Record &addMe);

    int GetLength();
    /*
     * Check if IsDirtyFlag is true then write the last page to the file
     * Add the new page to binary file.
     */
    void Add(Page &addMe);

    /* Retrieve the next record from the current Page. It checks if ISDirty flag
     * is true then write the records back to disk
     */

    int GetNext(Record &fetchMe);

    /* Retrieve the next record from the current Page that satisfy the CNF
     * passed to function. It checks if ISDirty flag is true then write the
     * records back to disk
     */
    int GetNext(Record &fetchMe, CNF &applyMe, Record &literal);

    /* Create the new file of type myType.
     */
    int Create(char *name, fType myType, void *startup);

    /* Open the SortedFile for reading and writing
     */
    int Open(char *name);

    /* Close the opened SortedFile
     */
    int Close();

    /* Load the binary file using the *.tbl file of tpc-h benchmark
     */
    void Load(Schema &mySchema, char *loadMe);

    /* Retrieve the whichPage
    */
    void GetPage (Page &putItHere, off_t whichPage);
};

#endif	/* _SortedFile_H */
