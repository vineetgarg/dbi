/************************************************************************************
File Name - BigQ.cc
Functionality - It is developed as milestone 1 of project2. It retrieves unsorted records
		from an input pipe, sort the records using TPMMS alogrithm and then
		write the sorted records back to output pipe
Limitations - 1. It provides one time access only i.e. if more than one thread try to access
	      	 it may create problem.
    	      2. It assumes that user have enough memory to all records in one run (whatever
		 the run length is) and there is enough memory to hold all records contained
		 in pages equal to number of runs (during 2nd phase). In short it do not create
		 any input or output buffer
Author - 	Vineet Garg
Date - 		5 Feb, 2011
*************************************************************************************/
#include "BigQ.h"
#include<algorithm>
#include<iostream>
#include<queue>
#include<set>

//Global Variables
//OrderMaker *myComparison = NULL; //to use in compare function

//Function declaration
static void* bigQWorker(void* bigQObj); //act as a bridge between BigQ constructor and sorting
static int Compare (Record*, Record*); //used in sorting phase 1

/************************************************************************************
Function - Compare
Parameters - Record, Record
Functionality - It implements compare to be used for sorting runs in first phase
		It uses compare of Comparison Engine
		It returns true(1) if r1 is small
*************************************************************************************/
int recordWrapperPhase1 :: compareR (const void *a, const void *b) {
	recordWrapperPhase1 *r1 = (recordWrapperPhase1 *)a;
	recordWrapperPhase1 *r2 = (recordWrapperPhase1 *)b;

	ComparisonEngine engine;

	int result = engine.Compare(&(r1->tempRecord), &(r2->tempRecord), r1->sortOrder);
	if(result < 0) {
		return 1;
	}
	else {
		return 0;
	}
}


class compareRecords {
public:
	int operator() (recordWrapper *rw1, recordWrapper *rw2 ) {
		ComparisonEngine engine;
		int compareStatus = engine.Compare( &(rw1->newRecord), &(rw2->newRecord), rw1->sortOrd );
		if (compareStatus < 0)
			return 1;
		else
			return 0;
	}
};
/************************************************************************************
Function - BigQ Constructor
Functionality - Initiliaze input & output pipe, run length, ordermaker
		It also initialized the temp file for storing sorted runs
*************************************************************************************/
BigQ :: BigQ (Pipe &inputPipe, Pipe &outputPipe, OrderMaker &sortOrder, int runLength):
	inputPipe(inputPipe),outputPipe(outputPipe),sortOrder(sortOrder),runLength(runLength)
{
	char *name = "bigQtemp";
	//char *fileName = strdup(name);
	
	int randNumber = (rand() % 100) + 1 ; //generate random number b/w 1 & 99
	snprintf(fileName, sizeof(fileName), "%s%d", name, randNumber);
	//itoa(randNumber, fileNumber, 10);

	//strcat(fileName, fileNumber);

	//cout<<"BigQ will use temp file: "<<fileName<<endl;

	file.Open(0, fileName); //open temp file to store sorted records	

	currentPageInRun = 0; //End page initialized to zero
	//myComparison = &sortOrder; //store sortOrder to use in sorting
	totalRuns = 0; //initialize total runs to zero

	pthread_create(&sortingThread, NULL, bigQWorker, (void *) this); //create new thread
}


/************************************************************************************
Function - BigQ destructor
Functionality - Wait for the thread responsible for sorting to finish, then close
		the temp file	
*************************************************************************************/
BigQ :: ~BigQ()
{
	file.Close(); //close the file 
	remove(fileName);
	pthread_join(sortingThread, NULL); //wait for the sorting thread to finish 
	
	#ifdef DEBUG
	cout<<"\nDestructing BigQ object.."<<endl;
	#endif
}


/************************************************************************************
Function - bigQWorker
Parameters - *bigQObj - pointer to BigQ object
Functionality - It starts sorting (Phase 1)
*************************************************************************************/
static void *bigQWorker(void *bigQObj)
{
	BigQ *bigQ = (BigQ *) bigQObj;
	bigQ -> sortingPhase1();

	pthread_exit(NULL);	 //kill the thread
}


/************************************************************************************
Function - sortingPhase1
Parameters - N/A
Functionality - It starts sorting (Phase 1) i.e. it get records from input pipe
		and divide them into run, then sorts each run using STL sort
*************************************************************************************/
void BigQ :: sortingPhase1() {

	#ifdef DEBUG
	cout<<"\nStarting sorting phase 1............."<<endl;
	#endif

	Record *Rec; // read from pipe and added to page which swallows it
	//Record *CopyRec; // this is kept for sorting purpose and stored in vector
	recordWrapperPhase1 *CopyRec;

	vector<recordWrapperPhase1*> recordVector; //to hold records equal to one run

	Page myPage;
	int numberOfPages = 0; //keeps track of how many pages worth records we have

	Rec = new Record; //to keep record

	while(inputPipe.Remove(Rec)) 
	{

		//make a copy of Rec since it will be destroyed after call to Append
		CopyRec = new recordWrapperPhase1;		
		(CopyRec->tempRecord).Copy(Rec);
		(CopyRec->sortOrder) = &sortOrder;

		if(!myPage.Append(Rec))		//Page is full
		{
			numberOfPages++;

			if(numberOfPages == runLength) //one run is complete. Need to sort it now
			{
				#ifdef DEBUG
				cout<<"Run Created.."<<endl;
				#endif

				sort(recordVector.begin(), recordVector.end(), recordWrapperPhase1::compareR); //changing to binding
				writeToDisk(recordVector);
				recordVector.clear();
				numberOfPages = 0;
			}

			myPage.EmptyItOut(); 
			myPage.Append(Rec);	//this append will never fail since page is emptied first
		}

		//put the rec into the list of recs to be quick-sorted as part of current run
		recordVector.push_back(CopyRec);
	}

	delete Rec;

	// Final write to disk
	if(recordVector.size() != 0) {

	#ifdef DEBUG
	cout<<"\nProcessing last run..."<<endl;
	#endif
		sort(recordVector.begin(), recordVector.end(), recordWrapperPhase1::compareR);
		writeToDisk(recordVector);
		
		/*for (int i=0; i<recordVector.size(); i++)
			delete recordVector[i]; //free the memory */
		recordVector.clear();
	}

	inputPipe.ShutDown();

	#ifdef DEBUG
	cout<<totalRuns<<" runs written to disk"<<endl;
	#endif

	sortingPhase2(); //begin phase 2 of sorting
}



/************************************************************************************
Function - writeToDisk()
Parameters - vector<Record*>&
Functionality - It writes the sorted runs to disk keeping track of start & end page of
		each run
*************************************************************************************/
void BigQ :: writeToDisk(vector<recordWrapperPhase1*> &recordVector) {
	#ifdef DEBUG
	cout<<"\nWriting run to disk............"<<endl;
	#endif

	totalRuns++; //new run
	runMetaData *rmd = new runMetaData;
	rmd->startPage = currentPageInRun; //keep track of start page

	#ifdef DEBUG
	cout<<"Start page is: "<<currentPageInRun<<endl;
	#endif
	

	Page myPage;

	vector<recordWrapperPhase1*>::iterator startIt = recordVector.begin();
	vector<recordWrapperPhase1*>::iterator endIt = recordVector.end();

	while(startIt != endIt) {
		if(!myPage.Append(&((*startIt)->tempRecord))) {	// Page is full
			//write page to disk file containing sorted runs
			file.AddPage(&myPage, currentPageInRun);
			
			#ifdef DEBUG
		//	cout<<"Page "<<currentPageInRun<<" appended in run "<<totalRuns<<endl;
			#endif
	
			currentPageInRun++;

			myPage.EmptyItOut();
			myPage.Append( &((*startIt)->tempRecord));	
		}
		startIt++; //keep moving
	}

	// Write last page
	file.AddPage(&myPage, currentPageInRun);

	#ifdef DEBUG
		cout<<"Last Page "<<currentPageInRun<<" appended in run "<<totalRuns<<endl;
	#endif

	myPage.EmptyItOut();

	rmd->endPage = currentPageInRun; //store the end page number of this run
	runMetaDataVec.push_back(rmd); //store the run meta data into a vector

	//cout<<"."<<flush;

	currentPageInRun++;

	#ifdef DEBUG
		cout<<"Finished writing..................\n"<<endl;
	#endif

}


/************************************************************************************
Function - sortingPhase2()
Functionality - It starts sorting (Phase 2). It keeps a vector containing first page 
		of each run. It uses 'multiset' to sort & merge each record
Note - 		Due to some problem in Priority Queue multiset is used
*************************************************************************************/
void BigQ::sortingPhase2()
{
	#ifdef DEBUG
	cout<<"\nStarting sorting phase 2...."<<endl;
	cout<<"\nFile contains "<<file.GetLength()<<" pages"<<endl;
	#endif

	exhaustedRuns = 0; //initialize exhausted runs to zero

	vector<pageWrapper*> firstPageVector; //store first page of each run
	pageWrapper *firstPage = NULL; //store first page

	int currentPageNumber = 0; //current page to get from file


	//this will fetch the first page of each run and will store it in vector
	for(int i=1; i<=totalRuns; i++) //go on for all runs
	{
		currentPageNumber = (runMetaDataVec[i-1])->startPage; //get the first page number from run meta data, note that vector starts from zero
	
		firstPage = new pageWrapper;
		file.GetPage( &(firstPage->newPage), currentPageNumber); //get the page in firstPage

		firstPage->currentPage = currentPageNumber;
		
		firstPageVector.push_back(firstPage); //store the first page in vector 
		
	}



	multiset<recordWrapper*, compareRecords> ms; //create multiset
	
	recordWrapper *tempRecWrap = NULL; //need recordWrapper to initialize the multiset

	for(int j=0; j<totalRuns; j++)
	{
		tempRecWrap = new recordWrapper; //allocated memory for new record wrapper
		if(((firstPageVector[j])->newPage).GetFirst( &(tempRecWrap->newRecord)) != 0) //get the first record
		{
			tempRecWrap->runPosition = (j+1); //insert the run number
			(tempRecWrap->sortOrd) = &(this->sortOrder); //insert the sort order;
			ms.insert(tempRecWrap);
		}

		else //error in getting record
		{
			cerr<<"Error! no first record found in sorted run !! please try again"<<endl;
			exit(0);
		}
	}

	#ifdef DEBUG
	cout<<"\nInitializing done..."<<endl<<endl;
	#endif

	int positionRun;

	int debug_Records = 0;


	recordWrapper *temporaryRecordWrapper;
	//Merging
	while( exhaustedRuns < totalRuns )
	{
		temporaryRecordWrapper  = *(ms.begin()) ; //get the smallest record
		ms.erase(ms.begin()); //delete the smallest record from multiset

		debug_Records++;
		
		positionRun = temporaryRecordWrapper->runPosition; //get the run to which record belongs to
		outputPipe.Insert( &(temporaryRecordWrapper->newRecord)); //insert the record into pipe, note that newRecord is zeroed out

		//recordWrapper *temporaryRecordWrapper = new recordWrapper; //allocate memory to hold new recordWrapper
		//cout<<"DEBUG::Recordwrapper allocated"<<endl;

		
		if((firstPageVector[positionRun-1]->newPage).GetFirst( &(temporaryRecordWrapper->newRecord) ) == 0) //page is empty
		{
			//get next page
			firstPageVector[positionRun-1]->currentPage++; //increment the current page to keep track of the number of pages received from record
			
			if(firstPageVector[positionRun-1]->currentPage <= runMetaDataVec[positionRun-1]->endPage ) //check to see if all the pages are processed in the current run
			{
				#ifdef DEBUG
				//cout<<"Getting next page for run "<<positionRun<<endl;
				#endif

				//get the next page
				file.GetPage(&(firstPageVector[positionRun-1]->newPage), firstPageVector[positionRun-1]->currentPage);
					
				//get the record from the page
				if( (firstPageVector[positionRun-1]->newPage).GetFirst( &(temporaryRecordWrapper->newRecord) ) == 0 ) //temporaryRecord contains record
				{
					cerr<<"Error! this page is empty !"<<endl;
					exit(0);
				}
				temporaryRecordWrapper->runPosition = positionRun; //it belongs to same run
				 
				//insert the record into multiset
				ms.insert(temporaryRecordWrapper);
			}
			
			else //all the pages in current run are processed
			{
				#ifdef DEBUG
				cout<<positionRun<<" run exhausted !"<<endl;
				#endif

				exhaustedRuns++; //becuase run is exhausted
				//getchar();

				//delete temporaryRecordWrapper; //delete this because it wasn't used because run was exhausted

				#ifdef DEBUG
				cout<<"Total "<<exhaustedRuns<<" runs are exhausted as of now "<<endl;
				#endif

				//cout<<"."<<flush;
			}

			//cout<<"."; //display it to let user be aware of processing
		}
		
		else //page is not empty and record received successfully
		{
			temporaryRecordWrapper->runPosition = positionRun; //it belongs to the same run

			//insert the record into queue
			ms.insert(temporaryRecordWrapper);

		}

		//cout<<".";
			
	}  
	//cout<<"BigQ::shuting down the pipe"<<endl;
	outputPipe.ShutDown(); // shutdown the pipe
				
}
