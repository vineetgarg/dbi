/************************************************************************************
File Name - BigQ.h
Functionality - It  contains definition of following classes
		- runMetaData (stores the meta data of each run)
		- BigQ (class responsible for initiating sorting)
		- pageWrapper (wrapper for first page of each run)
		- recordWrapper (wrapper for record to be used in multiset)
Author - 	Vineet Garg
Date - 		5 Feb, 2011
*************************************************************************************/
#ifndef _BIGQ_H_
#define _BIGQ_H_

#include "Pipe.h"
#include "Comparison.h"
#include <pthread.h>
#include <vector>
#include <queue>
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdio.h>

using namespace std;

//#define DEBUG //to switch debugging messages off undefine this

//this is to store metadata i.e. start page & end page of each run
class runMetaData {

public:
	int startPage, endPage;
};


class recordWrapperPhase1 {
public:
	/*int operator < (recordWrapperPhase1 *rhs) {
		ComparisonEngine engine;
		int result = engine.Compare(&(this->tempRecord), &(rhs->tempRecord), this->sortOrder);
		if(result < 0) {
			return 1;
		}
		else {
			return 0;
		}
	}  */
	static int compareR(const void *a, const void *b);
	Record tempRecord;
	OrderMaker *sortOrder;
};

//this is to store record from sorted run
class recordWrapper {

public:
	Record newRecord;
	int runPosition;
	OrderMaker *sortOrd;
	/*int operator < (recordWrapper *rhs) {
		ComparisonEngine engine;
		int result = engine.Compare(&(this->newRecord), &(rhs->newRecord), this->sortOrd);
		if(result < 0) {
			return 1;
		}
		else {
			return 0;
		}
	}*/  
};

class BigQ {

private:

	// Input pipe & Output pipe
	Pipe &inputPipe, &outputPipe;
	
	// Run length in pages
	int runLength;
	
	//keeps track where the last page number where run ends 
	vector<runMetaData*> runMetaDataVec;
	
	//end page number in Run
	int currentPageInRun;

	//total number of runs written
	int totalRuns;

	//exhaustive runs i.e. which are run out of records
	int exhaustedRuns;

	// File to be written
	File file;

	//Thread responsible for sorting
	pthread_t sortingThread;

public:
	//temp file to store records
	char fileName[12];
	// constructor
	BigQ (Pipe &inputPipe, Pipe &outputPipe, OrderMaker &sortOrder, int runLength);

	// destructor
	~BigQ();

	// Phase1 of sorting
	void sortingPhase1();

	//Phase2 of sorting
	void sortingPhase2();

	// Write runs to the disk
	void writeToDisk(vector<recordWrapperPhase1*>&);

	// Sort order
	OrderMaker& sortOrder;


};


//this is to store first page of each sorted run
class pageWrapper {

public:
	Page newPage; //stores the page from sorted run
	int currentPage; //keeps track of the current page of run
};



#endif
