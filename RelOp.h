#ifndef REL_OP_H
#define REL_OP_H

#include <iostream>
#include "Pipe.h"
#include "DBFile.h"
#include "Record.h"
#include "Function.h"
#include <pthread.h>
#include <stdio.h>
#include "BigQ.h"
#include <vector>
#include "debugFile.h"


#define PIPE_SIZE 100
#define BLOCK_SIZE 1000

static void *selectPipeMethod(void *); //method which does the actual work
static void *selectFileMethod(void *);
static void *projectMethod(void *); //this does the work for project relational operator
static void *DuplicateRemoval_Method(void *); //this does the work for Duplicate Removal operator
static void *WriteOut_Method(void *); //this does the work for Write Out operator
static void *sum_Method(void *); //this does the work for SUM operator
static void *groupBy_Method(void *); //this does the work for GroupBy operator
static void *join_method(void *); //this implements the sort join & block nested loop

class RelationalOp {
	public:
	// blocks the caller until the particular relational operator 
	// has run to completion
	virtual void WaitUntilDone () = 0;

	// tell us how much internal memory the operation can use
	virtual void Use_n_Pages (int n) = 0;
};

class SelectFile : public RelationalOp { 

private:
	 pthread_t selectFileThread;
	// Record *buffer;

public:
	Pipe *outputPipe;
	DBFile *dbFile;
	CNF *selectOp;
	Record *literalRecord;

	void Run (DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal);
	void WaitUntilDone ();
	void Use_n_Pages (int n);

};

class SelectPipe : public RelationalOp {

	pthread_t selectPipeThread; //becuase SelectPipe spawns new thread to do actual work
public:	
	//pointers because actual objects are known at later time than constructing objects
	Pipe *inputPipe, *outputPipe;
	Record *literalRecord;
	CNF *selectOp;

	void Run(Pipe &inPipe, Pipe &outPipe, CNF &selOp, Record &literal); //public because caller will call this method
	void WaitUntilDone(); //implementing virtual method in derived class. Public because caller will call this to wait
	void Use_n_Pages (int n);
};

/* Implements the project relational operator */
class Project : public RelationalOp { 
	pthread_t projectThread;

public:
	Pipe *inputPipe, *outputPipe;
	int *keepMeProject, numAttsInputProject, numAttsOutputProject;

	void Run (Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput);
	void WaitUntilDone ();
	void Use_n_Pages (int n);
};

/* Implements sort join & block nested join */
class Join : public RelationalOp {
	pthread_t join_thread;

public:
	Pipe *inPipeL, *inPipeR, *outPipe;
	CNF *selOp;
	Record *literal;
	int runLength;

	Join(); //constructor	
	void Run (Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selOp, Record &literal);
	void WaitUntilDone ();
	void Use_n_Pages (int n);
};

/* It removes the duplicate records in a relation */
class DuplicateRemoval : public RelationalOp {

	pthread_t DuplicateRemoval_thread;

public:
	DuplicateRemoval();
	Pipe *inPipe_DuplicateRemoval, *outPipe_DuplicateRemoval;
	Schema *mySchema_DuplicateRemoval;
	int runLength_DuplicateRemoval;

	void Run (Pipe &inPipe, Pipe &outPipe, Schema &mySchema);
	void WaitUntilDone ();
	void Use_n_Pages (int n);
};

/* Implements SUM operator */
class Sum : public RelationalOp {
	pthread_t sum_thread;

public:
	Pipe *inPipe_sum, *outPipe_sum;
	Function *computeMe_sum;

	void Run (Pipe &inPipe, Pipe &outPipe, Function &computeMe);
	void WaitUntilDone ();
	void Use_n_Pages (int n);
};

/* Implements GroupBy operator */
class GroupBy : public RelationalOp {
	pthread_t groupBy_thread;

public:
	GroupBy();

	Pipe *inPipe_groupBy, *outPipe_groupBy;
	OrderMaker *groupAtts_groupBy;
	Function *computeMe_groupBy;
	int runLength_groupBy; //default value is 100

	void Run (Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe);
	void WaitUntilDone ();
	void Use_n_Pages (int n);
};

/*Write records to a text file */
class WriteOut : public RelationalOp {
	pthread_t WriteOut_thread;

public:
	Pipe *inPipe;
	FILE *outFile;
	Schema *mySchema;

	void Run (Pipe &inPipe, FILE *outFile, Schema &mySchema);
	void WaitUntilDone ();
	void Use_n_Pages (int n);
};
#endif
