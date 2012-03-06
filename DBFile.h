 /* File:   DBFile.h
 * Author: Vineet Garg
 */

#ifndef _DBFILE_H
#define	_DBFILE_H

#include "Defs.h"
#include "File.h"
#include "GenericDBFile.h"
#include "HeapFile.h"
#include "SortedFile.h"

class DBFile {
    GenericDBFile *databaseFile;
public:
    /* Constructor for DBFile
     * Initialize all the instance variable
     */
    DBFile();

    /* Destructor for DBFile
     * Close all the external files or memory opened
     */
    ~DBFile();

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

    /* Open the dbFile for reading and writing
     */
    int Open(char *name);

    /* Close the opened dbFile
     */
    int Close();

    /* Load the binary file using the *.tbl file of tpc-h benchmark
     */
    void Load(Schema &mySchema, char *loadMe);
};
#endif	/* _DBFILE_H */
