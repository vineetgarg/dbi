#include "RelOp.h"

/* SelectPipe::Run()
 * It stores all arguments and then spawns a new thread to do actual working.
 */
void SelectPipe::Run(Pipe &inFile, Pipe &outPipe, CNF &selOp, Record &literal){
	printMsg("SelectPipe called....", "");

	inputPipe = &inFile;	
	outputPipe = &outPipe;
	selectOp = &selOp;
	literalRecord = &literal;

	pthread_create(&selectPipeThread, NULL, selectPipeMethod, (void *) this); //create new thread
}


/* *selectFileMethod(void *)
 *  This is excuted in a seperate thread created by SelectPipe's run().
 *  It take every record from the input pipe and put only those records into output pipe which takes the given CNF
 */
static void *selectPipeMethod(void *thisObj){
	printMsg("selectPipeMethod called...", "");
	int totalRec_debug=0, recMatched_debug=0;

	SelectPipe *selectPipeObj = (SelectPipe *)thisObj;
	ComparisonEngine cmpObj;
	Record *recordTemp = new Record;

	while((selectPipeObj->inputPipe)->Remove(recordTemp)) //while there are records in db file
	{
		totalRec_debug++;
		if(cmpObj.Compare(recordTemp, selectPipeObj->literalRecord, selectPipeObj->selectOp)) //if record takes CNF
		{
			(selectPipeObj->outputPipe)->Insert(recordTemp); //put the record into pipe
			recMatched_debug++;
		}
	}
	(selectPipeObj->outputPipe)->ShutDown(); //shutdown the output pipe

	printMsg("Select Pipe::Total Records: ", totalRec_debug);
	printMsg("Select Pipe::Records Matched: ", recMatched_debug);
}

/* SelectPipe::WaitUntilDone()
 * It wait for the selectPipeThread to complete 
 */
void SelectPipe::WaitUntilDone (){
	printMsg("SelectPipe:: Waiting...", "");
	pthread_join(selectPipeThread, NULL); //wait for the thread to finish 
}

/* This is not required */
void SelectPipe::Use_n_Pages (int runlen) { }


/* SelectFile::Run()
 * It stores all arguments and then spawns a new thread to do actual working.
 */
void SelectFile::Run(DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal){
	printMsg("SelectFile::Run called...", "");

	dbFile = &inFile;	
	outputPipe = &outPipe;
	selectOp = &selOp;
	literalRecord = &literal;

	pthread_create(&selectFileThread, NULL, selectFileMethod, (void *) this); //create new thread
}

/* *selectFileMethod(void *)
 *  This is excuted in a seperate thread created by SelectFile's run().
 *  It take every record from the db file and put only those records into output pipe which takes the given CNF
 */
static void *selectFileMethod(void *thisObj){
	printMsg("selectFileMethod called....", "");
	int recordMatched_debug = 0, totalRecords_debug=0;

	SelectFile *selectFileObj = (SelectFile *)thisObj;
	ComparisonEngine cmpObj;
	Record *recordTemp = new Record;

	while((selectFileObj->dbFile)->GetNext(*recordTemp)) //while there are records in db file
	{
		totalRecords_debug++;	
		//printMsg("selectFileMethod()", "Got new record from db file");
		if(cmpObj.Compare(recordTemp, selectFileObj->literalRecord, selectFileObj->selectOp)) //if db file record takes CNF
		{
			//printMsg("selectFileMethod()", "CNF takes record");
			(selectFileObj->outputPipe)->Insert(recordTemp); //put the record into pipe
			recordMatched_debug++;
		}
	}
	(selectFileObj->outputPipe)->ShutDown(); //shutdown the output pipe
	cout<<"."<<flush;

	//printMsg("selectFileMethod()", "Processing Done..");
	printMsg("SelectFile::Total records: ", totalRecords_debug);
	printMsg("SelectFile::Records Matched: ", recordMatched_debug);
}


/* SelectFile::WaitUntilDone()
 * It wait for the selectFileThread to complete 
 */
void SelectFile::WaitUntilDone (){
	printMsg("SelectFile:: ", "Waiting...");
	pthread_join(selectFileThread, NULL); //wait for the thread to finish 
}

/*********************PENDING***************/
void SelectFile::Use_n_Pages (int runlen) {}


/* Project::Run()
 * It stores all arguments and then spawns a new thread to do actual working.
 */
void Project::Run (Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput){
	printMsg("Project::Run called...............", "");

	inputPipe = &inPipe;
	outputPipe = &outPipe;
	keepMeProject = keepMe;
	numAttsInputProject = numAttsInput;
	numAttsOutputProject = numAttsOutput;

	pthread_create(&projectThread, NULL, projectMethod, (void*)this); //create new thread
}

/* projectMethod(void *)
 * This is excuted in a seperate thread spawned by Project's run method.
 * It takes records from input pipe and project them on the given attributes before putting them back into output pipe
*/
void *projectMethod(void *thisObj){
	printMsg("projectMethod called...........", "");
	int recProjected_debug=0;

	Project *projectObj = (Project *)thisObj;

	Record *tempRecord = new Record; //to use in processing
	while((projectObj->inputPipe)->Remove(tempRecord)) //while there are records in the input pipe
	{
		//printMsg("projectMethod()", "Removed record from the input pipe.......");
		tempRecord->Project(projectObj->keepMeProject, projectObj->numAttsOutputProject, projectObj->numAttsInputProject); //project the record on given attributes
		(projectObj->outputPipe)->Insert(tempRecord); //put the record into output pipe
		recProjected_debug++;
	}

	delete tempRecord;	
	(projectObj->outputPipe)->ShutDown(); //shutdown the output pipe
	cout<<"."<<flush;
	printMsg("Project:: Records Projected: ", recProjected_debug);
}

/*Project::WaitUntilDone()
 *It blocks the execution until projectThread is completed
 */
void Project::WaitUntilDone(){
	printMsg("Project::Waiting.........", "");
	pthread_join(projectThread, NULL); //wait for the thread to finish 
}


/****************PENDING***************/
void Project::Use_n_Pages (int runlen) { }

/* DuplicateRemoval() 
 * It initialize the run length to 100
 */
DuplicateRemoval :: DuplicateRemoval():runLength_DuplicateRemoval(100) {
	printMsg("DuplicateRemoval::Intialize the run length to: ", runLength_DuplicateRemoval);
}
/*DuplicateRemoval::Run()
 *It initialize the instance variables and spawn a new thread to do the acutal work
 */
void DuplicateRemoval::Run (Pipe &inPipe, Pipe &outPipe, Schema &mySchema){
	printMsg("DuplicateRemoval::Run called...........", "");
	inPipe_DuplicateRemoval = &inPipe;
	outPipe_DuplicateRemoval = &outPipe;
	mySchema_DuplicateRemoval = &mySchema;

	pthread_create(&DuplicateRemoval_thread, NULL, DuplicateRemoval_Method, (void*)this); //create new thread
}


/*DuplicateRemoval_Thread(void *)
 *It is executed in a seperate thread by Duplicate's run method.
  It removes the duplicates from input pipe before putting them into output pipe
  Basically it sorts the records using BigQ and then compare them to remove the duplicates.
  It uses the ordermaker based on all attributes
 */
void *DuplicateRemoval_Method(void *thisObj_DuplicateRemoval){
	printMsg("DuplicateRemoval_Method called.......", "");
	int recordKeptCount_debug = 0, recordDiscaredCount_debug = 0;

	DuplicateRemoval *DuplicateRemovalobj = (DuplicateRemoval *)thisObj_DuplicateRemoval;
	
	//get the sort order
	OrderMaker sortOrderMaker = OrderMaker(DuplicateRemovalobj->mySchema_DuplicateRemoval);
	Pipe outPipe_BigQ(1000);
	//int runLength_DuplicateRemoval = 100; //PENDING
	BigQ *bigQDuplicateRemoval;

	printMsg("DuplicateRemoval:: Sorting records with run length: ", DuplicateRemovalobj->runLength_DuplicateRemoval);
	bigQDuplicateRemoval = new BigQ(*(DuplicateRemovalobj->inPipe_DuplicateRemoval), outPipe_BigQ, sortOrderMaker, DuplicateRemovalobj->runLength_DuplicateRemoval);

	ComparisonEngine compareRecords;
	Record *recordInPipe, *recordToBeCompared;
	recordInPipe = new Record;
	recordToBeCompared = new Record;

	if(outPipe_BigQ.Remove(recordToBeCompared)) //Do it only if there is record
	{
		printMsg("DuplicateRemoval_Method", "Removed first record from the output Pipe");
		recordInPipe->Copy(recordToBeCompared); //keep the copy of the record to compare next record
		(DuplicateRemovalobj->outPipe_DuplicateRemoval)->Insert(recordToBeCompared); //put the first record in pipe, note that record is emptied out
		recordKeptCount_debug++;
	}

	//printMsg("DuplicateRemoval_Method", "Removing records from pipe");
	while(outPipe_BigQ.Remove(recordToBeCompared))
	{
		//printMsg("DuplicateRemoval_Method", "Removing records from bigq pipe");
		//compare the records
		if(compareRecords.Compare(recordToBeCompared, recordInPipe, &sortOrderMaker) != 0) //if records are not equal 
		{
			recordInPipe->Copy(recordToBeCompared); //keep the copy of the record
			(DuplicateRemovalobj->outPipe_DuplicateRemoval)->Insert(recordToBeCompared);
			recordKeptCount_debug++;
		}

		else
			recordDiscaredCount_debug++;
		//printMsg("DuplicateRemoval_Method", "EndRemoving records from bigq pipe");
		//if records different then keep the record and put the record into pipe
	}

	(DuplicateRemovalobj->outPipe_DuplicateRemoval)->ShutDown();
	cout<<"."<<flush;

	//printMsg("DuplicateRemoval_Method", "Duplicates removed. Exiting this thread");
	printMsg("DuplicateRemoval::Records Discared: ", recordDiscaredCount_debug);
	printMsg("DuplicateRemoval::Records Kept: ", recordKeptCount_debug);
}

/*DuplicateRemoval:WaitUntilDone()
 *It blocks the execution until DuplicateRemoval_Thread is completed
 */
void DuplicateRemoval::WaitUntilDone(){
	printMsg("DuplicateRemoval::Waiting...............", "");
	pthread_join(DuplicateRemoval_thread, NULL); //wait for the thread to finish 
}


/*Set the run length to sort the records*/
void DuplicateRemoval::Use_n_Pages (int runlen) {
	runLength_DuplicateRemoval = runlen;
	printMsg("DuplicateRemoval::Run Length set to: ", runLength_DuplicateRemoval);

}

/* WriteOut::Run(Pipe&, FILE*, Schema*)
 * It stores all the incoming parameters and then spawn a new thread WriteOut_thread 
 */
void WriteOut::Run(Pipe &inPipe, FILE *outFile, Schema &mySchema){
	printMsg("WriteOut::Run called...........", "");
	this->inPipe = &inPipe;
	this->outFile = outFile;
	this->mySchema = &mySchema;

	pthread_create(&WriteOut_thread, NULL, WriteOut_Method, (void*)this); //create new thread
}

/* WriteOut_Method(void*)
 * This is executed in a seperate thread by WriteOut's run method.
 * It removes record from input pipe and call record's write_to_file to write record to given text file
 * Assumption - assuming whoever called write out will close the file
 */
void *WriteOut_Method(void *thisObj){
	printMsg("WriteOut_Method called.................", "");
	int recordsWritten_debug = 0;
	
	WriteOut *writeOutObj = (WriteOut*)thisObj;
	Record *tempRecord = new Record;

	while ((writeOutObj->inPipe)->Remove(tempRecord))
	{
		tempRecord->Write_To_File(writeOutObj->outFile, writeOutObj->mySchema);
		recordsWritten_debug++;
	}
	cout<<"."<<flush;
	//fclose(writeOutObj->outFile); //close the file 

	printMsg("WriteOut::Number of records written: ", recordsWritten_debug); 
}


/*WriteOut::WaitUntilDone()
 *It blocks the execution until WriteOut_Thread is completed
 */
void WriteOut::WaitUntilDone(){
	printMsg("WriteOut::Waiting...................................", "");
	pthread_join(WriteOut_thread, NULL); //wait for the thread to finish 
}


/****************PENDING***************/
void WriteOut::Use_n_Pages (int runlen) { }

/* Sum::Run(Pipe&, Pipe&, Function&)
 * It stores all the incoming parameters and then spawn a new thread sum_thread 
 */
void Sum::Run(Pipe &inPipe, Pipe &outPipe, Function &computeMe){
	printMsg("Sum::Run called........................", "");

	inPipe_sum = &inPipe;
	outPipe_sum = &outPipe;
	computeMe_sum = &computeMe;

	pthread_create(&sum_thread, NULL, sum_Method, (void*)this); //create new thread
}


void *sum_Method(void *thisSum){
	printMsg("sum_Method called......................", "");
	int totalRecords_debug = 0; 
	//double totalSum = 0;

	Sum *sumObj = (Sum*)thisSum;
	Record tempRecord, newRecord;

	int intResult = 0 ;
	double doubleResult = 0, totalSum = 0;
	//Type resultType;

	while ((sumObj->inPipe_sum)->Remove(&tempRecord)){
		totalRecords_debug++;

		(sumObj->computeMe_sum)->Apply(tempRecord, intResult, doubleResult);  	
		totalSum += (intResult + doubleResult); //one of them would be zero
	//	cout<<"total sum as of record "<<totalRecords_debug<<" is: "<<totalSum<<endl; //Debugging

		intResult = doubleResult = 0; //to make it safe for next iteration
	}

	printMsg("Sum::Total records processed: ", totalRecords_debug);
	printMsg("Sum:: Total sum is: ", totalSum);

	Attribute DS = {(char*)"sum", Double}; //create new attribute, assuming type to be double
	Schema totalSum_schema ("total_sum", 1, &DS); //create new in memory schema
	
	char rec[50]; //assuming that sum won't be more than 48 digits
	sprintf(rec, "%g|", totalSum); //write the sum into rec string;
	newRecord.ComposeRecord(&totalSum_schema, rec); //create new record;

	(sumObj->outPipe_sum)->Insert(&newRecord); //insert the record
	(sumObj->outPipe_sum)->ShutDown();

	cout<<"."<<flush;

}

/*Sum::WaitUntilDone()
 *It blocks the execution until sum_thread is completed
 */
void Sum::WaitUntilDone(){
	printMsg("Sum::Waiting.........................", "");
	pthread_join(sum_thread, NULL); //wait for the thread to finish 
}


/****************PENDING***************/
void Sum::Use_n_Pages (int runlen) {}

/* GroupBy Constructor
 * It initiliazes run length to 100 which later could be used to sort records
 */
GroupBy :: GroupBy(): runLength_groupBy(100) {
	printMsg("GroupBy::Initialize run length to: ", runLength_groupBy);
}

/* GroupBy::Run(Pipe, Pipe, OrderMaker, Function)
 * It initialize all the parameters and then spawn a new thread groupBy_thread
 */
void GroupBy::Run (Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe) { 
	printMsg("GroupBy::Run Called..................................", "");

	inPipe_groupBy = &inPipe;
	outPipe_groupBy = &outPipe;
	groupAtts_groupBy = &groupAtts;
	computeMe_groupBy = &computeMe;

	pthread_create(&groupBy_thread, NULL, groupBy_Method, (void*)this); //create new thread
}

/* groupBy_method(void*) 
 * It implements the Group By relational operator 
 */
void *groupBy_Method(void *thisGroupBy){
	printMsg("groupBy_Method called..............................", "");
	int totalRecords_debug=0, recGrouped_debug=0;

	GroupBy *groupByObj = (GroupBy*)thisGroupBy;
	Pipe sortedPipe(PIPE_SIZE);
	
	printMsg("GroupBy::Sorting records with run length: ", groupByObj->runLength_groupBy); //for debugging
	BigQ sort_groupBy(*(groupByObj->inPipe_groupBy), sortedPipe, *(groupByObj->groupAtts_groupBy), groupByObj->runLength_groupBy); //sort the records

	Record currentRecord, nextRecord, newRecord, tempRecord;
	double totalSum = 0, doubleResult=0;
	int intResult=0, numAttsRight, *attsToKeepRight, numAttsToKeep, numAttsInRecord;
	ComparisonEngine compareRecords;



	//Prepare sorted list for merging records(MergeRecords (Record *left, Record *right, int numAttsLeft,int numAttsRight, int *attsToKeep, int numAttsToKeep, int startOfRight);
	//First attribute is sum and next attributes are of order maker
	attsToKeepRight = (groupByObj->groupAtts_groupBy)->attsList(); //attributes to keep in given record
	numAttsRight = (groupByObj->groupAtts_groupBy)->numOfAtts(); //number of attributes to be kept 
	//cout<<"List of attributes to keep (from ordermaker) "<<endl;
	//for(int i=0; i<numAttsRight; i++)
	//	cout<<attsToKeepRight[i]<<" ";

	numAttsToKeep = numAttsRight + 1;	
	int *attsToKeep = new int[numAttsToKeep];
	attsToKeep[0] = 0;

	for(int j=1; j<numAttsToKeep; j++)
		attsToKeep[j] = attsToKeepRight[j-1];

	/*cout<<"List of attributes to keep (final list): "<<endl;
	for(int k=0; k<numAttsToKeep; k++)
		cout<<attsToKeep[k]<<" "; */

	Attribute DS = {(char*)"sum", Double}; //create new attribute, assuming type to be double
	Schema totalSum_schema ("total_sum", 1, &DS); //create new in memory schema
	char rec[70]; //assuming that sum won't be more than 68 digits
	
	if(sortedPipe.Remove(&currentRecord)){ //if there exists record
		(groupByObj->computeMe_groupBy)->Apply(currentRecord, intResult, doubleResult);  	
		totalSum += (intResult + doubleResult); //one of them would be zero
		numAttsInRecord = currentRecord.numOfAttInRecord(); //number of attributes in the record
		intResult = doubleResult = 0;
		totalRecords_debug++;
	}

	printMsg("GroupBy::Number of attributes in the record: ", numAttsInRecord); //debugging


	while(sortedPipe.Remove(&nextRecord)){
		if( (compareRecords.Compare(&currentRecord, &nextRecord, groupByObj->groupAtts_groupBy)) != 0){ //records are not equal
			printMsg("GroupBy::Unequal Records! Total sum as of now is: ", totalSum);

			snprintf(rec, 69, "%g|", totalSum); //write the sum into rec string;
			tempRecord.ComposeRecord(&totalSum_schema, rec); //create new record;
			newRecord.MergeRecords(&tempRecord, &currentRecord, 1, numAttsInRecord, attsToKeep, numAttsToKeep, 1); //merge the record 

			(groupByObj->outPipe_groupBy)->Insert(&currentRecord);
			currentRecord.Copy(&nextRecord); //save the next record
			totalSum = 0;
			recGrouped_debug++;
		}

		totalRecords_debug++;
		//cout<<"GroupBy: Record removed - "<<totalRecords_debug<<endl;

		(groupByObj->computeMe_groupBy)->Apply(nextRecord, intResult, doubleResult);  	
		totalSum += (intResult + doubleResult); //one of them would be zero
		intResult = doubleResult = 0;
	}
			
	snprintf(rec, 69, "%g|", totalSum); //write the sum into rec string;
	tempRecord.ComposeRecord(&totalSum_schema, rec); //create new record;
	newRecord.MergeRecords(&tempRecord, &currentRecord, 1, numAttsInRecord, attsToKeep, numAttsToKeep, 1); //merge the record 
	recGrouped_debug++;

	(groupByObj->outPipe_groupBy)->Insert(&currentRecord);
	(groupByObj->outPipe_groupBy)->ShutDown();

	printMsg("GroupBy::Total records : ", totalRecords_debug);
	printMsg("GroupBy::Total records after group by: ", recGrouped_debug);
	cout<<"."<<flush;

}

/* GroupBy::WaitUntilDone()
 * It blocks the execution until groupBy_thread is completed
 */
void GroupBy::WaitUntilDone(){
	printMsg("GroupBy::Waiting.....................................................", "");
	pthread_join(groupBy_thread, NULL); //wait for the thread to finish 
}


/* GroupBy::Use_n_Pages(int)
 * set the run length
 */
void GroupBy::Use_n_Pages (int runlen) {
	runLength_groupBy = runlen;
	printMsg("GroupBy::Set the run length to: ", runLength_groupBy);
}

/* Join()
 * It initializes the default run length to 100
 */
Join :: Join(): runLength(100) {
	printMsg("Join::Initialize run length to: ", runLength );
}

/* Join::Run(Pipe&, Pipe&, Pipe&, CNF&, Record&)
 * It initializes all the parameters and spawn a new thread join_thread to do the actual work
 */
void Join :: Run (Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selOp, Record &literal){
	printMsg("Join::Run Called...........................................", "");

	this->inPipeL = &inPipeL;
	this->inPipeR = &inPipeR;
	this->outPipe = &outPipe;
	this->selOp = &selOp;
	this->literal = &literal;

	pthread_create(&join_thread, NULL, join_method, (void*)this); //create new thread
}

/* Join :: join_method()
 * It is executed in a seperate thread join_thread spawned by Join::Run()
 * It implement sort join & block nested join based upon CNF
 */
void *join_method(void *thisJoin){
	printMsg("join_method called.........................................", "");
	int numRecL_debug=0, numRecR_debug=0, numRecJoined_debug=0;
	//Schema supp ("catalog", "supplier");
	//Schema partsup ("catalog", "partsupp");

	Join *joinObj = (Join *)thisJoin;
	
	OrderMaker sortOrderL, sortOrderR;
	int sort_join;

	//sort_join will decide if sort-merge is to be implemented or block-nested	
	sort_join = (joinObj->selOp)->GetSortOrders(sortOrderL, sortOrderR);

	if(sort_join){
		printMsg("Join::Executing sort join..", "");
		//printMsg("Join::Sort Order for Left Pipe: ", sortOrderL.Print();
		//cout<<"Sort Order for Right Pipe: "<<endl;
		//	sortOrderR.Print();

		Pipe sortedPipeL(PIPE_SIZE), sortedPipeR(PIPE_SIZE);
		BigQ sortL(*(joinObj->inPipeL), sortedPipeL, sortOrderL, joinObj->runLength); //sort the left pipe
		BigQ sortR(*(joinObj->inPipeR), sortedPipeR, sortOrderR, joinObj->runLength); //sort the right pipe

		Record recL, recR, *tempRec, mergedRecord;
		ComparisonEngine comp;
		vector<Record*> recVector;
		
		//1 - get next record, 0-do not get next record, -1 - pipe is empty
		int getNextL=1, getNextR=1, compareStatus, numAttsLeft, numAttsRight, *attsToKeep, numAttsToKeep, startOfRight;
		int zeroRecL, zeroRecR;

		zeroRecL = sortedPipeL.Remove(&recL);
		numRecL_debug++;
		zeroRecR = sortedPipeR.Remove(&recR);
		numRecR_debug++;

		getNextR = getNextL = 0;

		if ( zeroRecL == 0 || zeroRecR == 0 ) { //if either of the pipe is emtpy
			printMsg("Join::One of the pipe is empty! So exiting thread", "");
			(joinObj->outPipe)->ShutDown();
			pthread_exit(NULL);
		}


		//create parameters for merging records
		numAttsLeft = recL.numOfAttInRecord();
		numAttsRight = recR.numOfAttInRecord();
		printMsg("Join::Number of atts in left record: ", numAttsLeft);
		printMsg("Join::Number of atts in Right record: ", numAttsRight );
		numAttsToKeep = numAttsLeft + numAttsRight;
		startOfRight = numAttsLeft;

		attsToKeep = new int[numAttsToKeep];
		for (int i=0; i<numAttsToKeep; i++) 
			attsToKeep[i] = i;

		for (int j=numAttsLeft; j<numAttsToKeep; j++)
			attsToKeep[j] = (j-numAttsLeft);

		printMsg("Join::Total num of atts to keep: ", numAttsToKeep );
		printMsg("Join::Start of right: ", startOfRight<<endl );
		/*cout<<"Attributes to keep: ";
		for(int k=0; k<numAttsToKeep; k++)
			cout<<" "<<attsToKeep[k];
		cout<<endl; */

		while(1){
			if (getNextL == -1 || getNextR == -1)
				break;
			if (getNextL == 1){ //get next record from left Pipe
				if( !(sortedPipeL.Remove(&recL)) ){ //it could be empty here
					printMsg("Join:: PipeL is empty! Breaking operation", "");		
					break; //no need to move forward if pipe is empty, just break the whole operation
				}

				numRecL_debug++;
			}
			if (getNextR == 1){ //get next record from right Pipe
				if ( !(sortedPipeR.Remove(&recR)) ){ //it could be empty here
					printMsg("Join::PipeR is empty! Breaking operation", "");
					break;
				}
				numRecR_debug++;
			}
			compareStatus =  comp.Compare(&recL, &sortOrderL, &recR, &sortOrderR);

			if(compareStatus < 0){ //recL < recR
				//discard recL
				getNextL = 1;
				getNextR = 0;
			}

			else if (compareStatus > 0){ //recL > recR
				//discare recR
				getNextL = 0;
				getNextR = 1;
			}

			else{ //recL = recR
				int recInVector_debug = 0;
				//get all the records from right pipe which matches the current record from left pipe
				while ( comp.Compare(&recL, &sortOrderL, &recR, &sortOrderR) == 0 ){
					tempRec = new Record;
					tempRec->Copy(&recR); //keep the copy of right pipe's record
					recVector.push_back(tempRec);
					recInVector_debug++;
					if( !(sortedPipeR.Remove(&recR)) ){ //if right pipe is empty
						getNextR = -1;
						break;
					}
					numRecR_debug++;
				}
				int vecPos = 0;
				
				while ( comp.Compare(&recL, &sortOrderL, recVector[vecPos], &sortOrderR) == 0 ) {
					if( comp.Compare(&recL, recVector[vecPos], joinObj->literal, joinObj->selOp) != 0 ){ //if CNF takes these records
						//merge the records
						mergedRecord.MergeRecords (&recL, recVector[vecPos], numAttsLeft, numAttsRight, attsToKeep, numAttsToKeep, startOfRight);
						//put into output pipe
						(joinObj->outPipe)->Insert(&mergedRecord);
						numRecJoined_debug++;

					}

					vecPos++;
					if ( !(vecPos < recVector.size()) ){ //all records in vector are proecessed 
						vecPos = 0;
						if( !(sortedPipeL.Remove(&recL)) ) { //get next rec from left pipe
							getNextL = -1; //left pipe is empty
							break;
						}
						numRecL_debug++;
					}
				} //at this point either recL has record or last while was broken
				
				for (int pos=0; pos<recVector.size(); pos++)
					delete recVector[pos]; //because i think calling clear won't deallocate this memory
				recVector.clear();

				if( getNextL != -1 && getNextR != -1 ) { //if neither of the pipes are empty
					//do nothing because we recL and recR already has new record	
					getNextL = 0;
					getNextR = 0; 
				}

			}
			
		}
		delete attsToKeep;

		printMsg("Join::Total records from Left pipe: ", numRecL_debug);
		printMsg("Join::Total Records from right pipe: ", numRecR_debug);
		printMsg("Join::Total records after joining: ", numRecJoined_debug);
		(joinObj->outPipe)->ShutDown();
		pthread_exit(NULL);

	}

	else{
		printMsg("Join::Executing block nested join..", "");
		int recLeft_debug=0, recRight_debug=0, recMerged_debug=0;
		
		//create the db file (random file name)
		char *name = "block.nested";
		int randNum = (rand() % 100 );
		char fileName[20];
		snprintf(fileName, sizeof(fileName), "%s%d", name, randNum);

		printMsg("Join::records will be stored at temporary file: ", fileName);	
		DBFile tempDBFile;
		tempDBFile.Create(fileName, heap, NULL);

		Record recL, recR, mergedRecord;
		ComparisonEngine compareRec;

		//store all the records of Lelf Pipe on disk
		//tempDBFile.MoveFirst();
		while ((joinObj->inPipeL)->Remove(&recL)) 
			tempDBFile.Add(recL);
		
		//create memory block
		int blockSize = (joinObj->runLength) * 610; //assuming 610 records in a page
		printMsg("Join::Left block size: ", blockSize);
		Record *recLBlock = new Record[blockSize];
		int counterL=0;

		Record recRBlock[BLOCK_SIZE]; //to store the records from right pipe
		int counterR=0;
		bool innerLoopBreak = false, outerLoopBreak=false;

		int numAttsLeft, numAttsRight, *attsToKeep, numAttsToKeep, startOfRight;
		tempDBFile.Close();
		tempDBFile.Open(fileName);
		tempDBFile.MoveFirst();
		while (1) {
			//get a block from right pipe	
			while (counterR < BLOCK_SIZE) {
				if( (joinObj->inPipeR)->Remove(&recRBlock[counterR]) == 0 ) { //if pipe is empty then break out of loop
					outerLoopBreak = true;
					break;
				}
			//recRBlock[counterR].Print(&partsup); //debug
			counterR++;
			recRight_debug++;
			}

			//cout<<"Join(): Retrieved record from right Pipe: "<<counterR<<endl;
		
			while (1) {
				while( counterL < blockSize ) {
					if( (tempDBFile.GetNext(recLBlock[counterL]) == 0 ) ) { //if file is empty
						innerLoopBreak = true;
						break;
					}
				//recLBlock[counterL].Print(&supp); //debug
				counterL++;
				recLeft_debug++;
				}

				//compare the records
				for(int j=0; j<counterR; j++) {
					for(int i=0; i<counterL; i++) {
						if(compareRec.Compare(&(recLBlock[i]), &(recRBlock[j]), joinObj->literal, joinObj->selOp)){
							numAttsRight = recRBlock[j].numOfAttInRecord();
							numAttsLeft = recLBlock[i].numOfAttInRecord();
							numAttsToKeep = numAttsLeft + numAttsRight;
							startOfRight = numAttsLeft;
							attsToKeep = new int[numAttsToKeep];

							for (int pos=0; pos<numAttsToKeep; pos++) 
								attsToKeep[pos] = pos;

							for (int at=numAttsLeft; at<numAttsToKeep; at++)
								attsToKeep[at] = (at-numAttsLeft);

							mergedRecord.MergeRecords (&recLBlock[i], &recRBlock[j], numAttsLeft, numAttsRight, attsToKeep, numAttsToKeep, startOfRight);
							//put into output pipe
							(joinObj->outPipe)->Insert(&mergedRecord);
							recMerged_debug++;
						}

					}
				}

				counterL = 0;

				if(innerLoopBreak){
					tempDBFile.MoveFirst(); //start again
					cout<<"."<<flush;
					break; //break the inner while loop
				}

		 	}

			innerLoopBreak = false;
			counterR=0;
			if (outerLoopBreak)
				break; //break the outer while loop
		}

		printMsg("Join::Records from left pipe: ", recLeft_debug);
		printMsg("Join::Records from right Pipe: ", recRight_debug );
		printMsg("Join::Total records after joining: ", recMerged_debug );

		//delete recLBlock;
		//shutdown the pipe
		(joinObj->outPipe)->ShutDown();

		//close the file
		tempDBFile.Close();
		//remove the file
		remove( fileName );

	}

	cout<<"."<<flush;

}

void Join::WaitUntilDone(){
	printMsg("Join::Waiting.......................................", "");
	pthread_join(join_thread, NULL); //wait for the thread to finish 
	
}


/* GroupBy::Use_n_Pages(int)
 * set the run length
 */
void Join::Use_n_Pages (int runlen) {
	runLength = runlen;
	printMsg("Join::Set the run length to: ", runLength );
}
