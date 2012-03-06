/* 
 * File:   HeapFile.h
 */

#ifndef _HeapFile_H
#define	_HeapFile_H

#include "Defs.h"
#include "File.h"
#include "GenericDBFile.h"
#include "CommonClass.h"

class HeapFile: virtual public GenericDBFile {
    // fp points to binary file
    File fp;

    //Current page object to perform subsequent getNext() and add() operations
    Page currentPage;
    // page number to retrieve next page when you run out of page while doing getNext()
    int pageNumber;
    // Flag to indicate whether the current page has pending transaction or not
    bool isDirty;
    /* Flag to indicate whether the current page that we writing is a last page
     * that we retrieved earlier to append more records or is it new page
     */
    int pendingLastPage;
    /*private function to checck if ISDirty flag is true then write the record
     * back to disk.
     */
    HeaderInformation headerInformation;
    bool checkISDirty();
public:
    /* Constructor for HeapFile
    * Initialize all the instance variable
    */
    HeapFile ();

    /* Destructor for HeapFile
    * Close all the external files or memory opened
    */
    ~HeapFile ();

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

    /* Open the HeapFile for reading and writing
     */
    int Open(char *name);

    /* Close the opened HeapFile
     */
    int Close();
    
    /* Load the binary file using the *.tbl file of tpc-h benchmark
     */
    void Load(Schema &mySchema, char *loadMe);

    /* Retrieve the whichPage
    */
    void GetPage (Page &putItHere, off_t whichPage);
};

#endif	/* _HeapFile_H */
