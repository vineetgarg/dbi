/* File - Statistics.cc
 * @Vineet Garg
 * 4 April, 2011
 */
#include "Statistics.h"

/* attContainer()
 * default constructor
 */
attContainer::attContainer() {}

/* attContainer(string, int)
 * this constructor is to create attribute container with given att_name & numDistincts
 */
attContainer::attContainer(string att_name, int numDistincts) {
	this->att_name = att_name;
	this->numDistincts = numDistincts;

	printMsg("attContainer::Created new attribute...", "");
	printMsg("	att_name: ", att_name);
	printMsg("	numDistincts: ", numDistincts);
	printMsg("", '\n');
}

/* attContainer(attContainer&)
 * Copy constructor of attribute container
 */
attContainer::attContainer(const attContainer &copyMe) {
	printMsg("attContainer::Copy constructor..", "");
	this->att_name = copyMe.att_name;
	this->numDistincts = copyMe.numDistincts;
}

/* set_att_name(string)
 * to set the attribute name in attribute container
 */
void attContainer::set_att_name(string att_name) {
	this->att_name = att_name;
}

/* get_att_name()
 * return the name of attribute in attribute container
 */
string attContainer::get_att_name() {
	return att_name;
}

/* set_numDistincts(int)
 * to set the value of distinct number of tuples in attribute
 */
void attContainer::set_numDistincts(int numDistincts) {
	this->numDistincts = numDistincts;
}

/* int get_numDistincts()
 * returns the number of distincts tuples of attribute
 */
int attContainer::get_numDistincts() {
	return numDistincts;
}

/* attContainer::print()
 * It print the content of attribute
 */
void attContainer::print() {
	cout<<"Contents of attribute...."<<'\n';
	cout<<"		att_name:	"<<att_name<<endl;
	cout<<"		numDistincts:	"<<numDistincts<<endl;
	cout<<endl;
}

/* void Write(FILE *)
 * It writes the content of attribute on file
 */
void attContainer::Write(FILE *fileObj) {
	int attSize = att_name.size();

	fwrite( &attSize, sizeof(int), 1, fileObj);
	fwrite( att_name.c_str(), sizeof(char), attSize, fileObj);
	fwrite( &numDistincts, sizeof(int), 1, fileObj);
}

/* void Read(FILE *)
 * It reads the content of attribute from file
 */
void attContainer::Read(FILE *fileObj) {
	printMsg("attContainer::Read - Reading from file..", "");

	int attSize = 0;

	fread( &attSize, sizeof(int), 1, fileObj);

	char *buffer = new char[attSize + 1];
	fread( buffer, sizeof(char), attSize, fileObj);
	fread( &numDistincts, sizeof(int), 1, fileObj);
	att_name = buffer;

	printMsg("attContainer::Read - read att_name: ", att_name );
}

/* relContainer()
 * default constructor for relation container. It initializes the number of attributes to zero
 */
relContainer::relContainer():numAtt(0) {}

/* relContainer(string, int, int)
 * This constructor creates relation container with given relation name, number of tuples and partition number
 */
relContainer::relContainer(string rel_name, int numTuples, int partition_num) {
	this->rel_name = rel_name;
	this->numTuples = numTuples;
	this->partition_num = partition_num;
	this->numAtt = 0;

	printMsg("relContainer::Created new relation...", "");
	printMsg("	rel_name: ", rel_name);
	printMsg("	numTuples: ", numTuples);
	printMsg("	partition_num: ", partition_num);
	printMsg("	numAtt: ", numAtt);
	printMsg("", '\n');
}

/* relContainer(relContainer &copyMe)
 * Copy constructor for relation container. Copy everything including name
 */
relContainer::relContainer(const relContainer &copyMe) {
	printMsg("relContainer::relContainer - Copy Constructor", "");
	this->numTuples = copyMe.numTuples;
	this->partition_num = copyMe.partition_num;
	this->numAtt = copyMe.numAtt;
	this->rel_name = copyMe.rel_name;
	
	//copy whole att_map
	attMap::const_iterator startIt, endIt;
	startIt = (copyMe.att_map).begin();
	endIt = (copyMe.att_map).end();

	//attContainer *tempAtt;

	while( startIt != endIt ){
		attContainer *tempAtt = new attContainer( *(startIt->second) );
		att_map[tempAtt->get_att_name()] = tempAtt;
	//	*tempAtt = *(startIt->second);
		startIt++;
	}
}

/* ~relationContainer()
 * Destructor for relation container. It deletes all the memory occupied to attribute container in att_map
 */
relContainer::~relContainer() {
	printMsg("relContainer::Destructing: ", rel_name);

	attMap::iterator startIt, endIt;
	startIt = att_map.begin();
	endIt = att_map.end();

	for( ; startIt != endIt; ++startIt )
		delete startIt->second;
}

/* set_rel_name(string)
 * this set the name of relation in relation container
 */
void relContainer::set_rel_name(string rel_name) {
	this->rel_name = rel_name;
}

/* string get_rel_name()
 * this returns the relation name of the relation container
 */

string relContainer::get_rel_name() {
	return rel_name;
}

/* void set_numTuples(int)
 * this set the name of tuples in relation container
 */
void relContainer::set_numTuples(int numTuples) {
	this->numTuples = numTuples;
}

/* int get_numTuples()
 * this returns the relation name of the relation container
 */
int relContainer::get_numTuples() {
	return numTuples;
}

/* void set_partition_num(int)
 * this set the partition number the relation belongs to
 */
void relContainer::set_partition_num(int partition_num) {
	this->partition_num = partition_num;
}

/* int get_partition_num()
 * this returns the partition number of the relation it belongs to
 */
int relContainer::get_partition_num() {
	return partition_num;
}

/* void set_numAtt(int)
 * This set the number of attributes in a relation 
 */
void relContainer::set_numAtt(int numAtt) {
	this->numAtt = numAtt;
}

/* int get_numAtt()
 * this returns the number of attribues in the relation
 */
int relContainer::get_numAtt() {
	return numAtt;
}

/* relContainer::print()
 * It print the contents of the relation
 */
void relContainer::print() {
	cout<<"Contents of Relation...."<<'\n';
	cout<<"		rel_name:	"<<rel_name<<endl;
	cout<<"		numTuples:	"<<numTuples<<endl;
	cout<<"		partition_num:	"<<partition_num<<endl;
	cout<<"		numAtt:	"<<numAtt<<endl;

	attMap::const_iterator startIt, endIt;
	startIt = att_map.begin();
	endIt = att_map.end();

	attContainer *tempAtt;

	while( startIt != endIt ){
		tempAtt =  (startIt->second) ;
		tempAtt->print();
		startIt++;
	}
	cout<<endl;
}

/* void Write()
 * This write the content of the relation on file
 */
void relContainer::Write(FILE *fileObj) {
	printMsg("relContianer::Write-Writing contents on file....", "");

	int relSize = rel_name.size();

	fwrite(&relSize, sizeof(int), 1, fileObj);
	fwrite(rel_name.c_str(), sizeof(char), relSize, fileObj);
	printMsg("relContianer::Write - rel_name: ", rel_name.c_str());

	fwrite(&numTuples, sizeof(int), 1, fileObj);
	fwrite(&partition_num, sizeof(int), 1, fileObj);
	fwrite(&numAtt, sizeof(int), 1, fileObj);

	attMap::const_iterator startIt, endIt;
	startIt = att_map.begin();
	endIt = att_map.end();

	for( ; startIt != endIt; ++startIt)
		(startIt->second)->Write(fileObj);
}

/* void Read()
 * This read the content of the relation from file
 */
void relContainer::Read(FILE *fileObj) {
	printMsg("relContainer::Read - Reading from file....", "");

	int relSize = 0;
	/*fileObj.read( (char *) &temp, sizeof(int) );
	
	printMsg("relContainer::Read - temp size : ", temp);
	char *buffer = new char[temp + 1];
	fileObj.read( (char *) &buffer, temp );
	rel_name = buffer;

	fileObj.read( (char *) &numTuples, sizeof(int) );
	fileObj.read( (char *) &partition_num, sizeof(int) );
	fileObj.read( (char *) &numAtt, sizeof(int) ); */
	
	/*fscanf(fileObj, "%d", &relSize );
	char *buffer = new char[relSize + 1];
	fscanf(fileObj, "%s", buffer );
	fscanf(fileObj, "%d", &numTuples);
	fscanf(fileObj, "%d", &partition_num);
	fscanf(fileObj, "%d", &numAtt);
	rel_name = buffer; */
	
	fread(&relSize, sizeof(int), 1, fileObj);

	printMsg("relContianer::Read- relSize: ", relSize);
	char *buffer = new char[relSize + 1];
	fread(buffer, 1, relSize, fileObj);
	fread(&numTuples, sizeof(int), 1, fileObj);
	fread(&partition_num, sizeof(int), 1, fileObj);
	fread(&numAtt, sizeof(int), 1, fileObj);

	printMsg("relContianer::Read- buffer: ", buffer);
	rel_name = buffer;
	
	printMsg("relContainer::Read - rel_name: ", rel_name);
	for( int i=0; i<numAtt; i++) {
		attContainer *tempAtt = new attContainer();
		tempAtt->Read(fileObj); //read the attribute
		att_map[tempAtt->get_att_name()] = tempAtt; //put the attribute into unordered map
	} 
}

/* Statistics()
 * default constructor of Statistics class. It initialize the number of relations to zero
 */
Statistics::Statistics(): numRelations(0) {
	printMsg("Statistics::Create new Statistics object...", "");
	printMsg("	numRelations: ", numRelations);
	printMsg("", '\n');
} //initialize number of relations to zero

/* Statistics(Statistics &)
 * Copy constructor, copy exactly everything
 */
Statistics::Statistics(Statistics &copyMe)
{
	printMsg("Statistics::Copy Constructor...", "");

	this->numRelations = copyMe.numRelations;
	
	relMap::const_iterator startIt, endIt;
	startIt = (copyMe.rel_map).begin();
	endIt = (copyMe.rel_map).end();

	//relContainer *tempRelC;
	for(; startIt != endIt; ++startIt ) {
		relContainer *tempRelC = new relContainer( *(startIt->second) ); //create new copy of relation container
		rel_map[tempRelC->get_rel_name()] = tempRelC;
	}

}

/* ~Statistics()
 *  Delallocate all the memory allocated by rel_map
 */
Statistics::~Statistics()
{
	//Delete all the relContainer in rel_map
	printMsg("Statistics::Destructing....", "");

	relMap::iterator startIt, endIt;
	startIt = rel_map.begin();
	endIt = rel_map.end();

	for( ; startIt != endIt; ++startIt)
		delete startIt->second;
}

/* void AddRel(char*, int)
 * This Add a new relation or updates the exisiting relations with num Tuples
 */
void Statistics::AddRel(char *relName, int numTuples) 
{
	if( relName == NULL || ((string)relName).empty() ) {
		cerr<<"Statistics::AddRel-relName is empty or NULL! Exiting....";
		exit(0);
	}

	relMap::iterator tempIt;
	tempIt = rel_map.find(relName); //find if the relation already exists or not

	if ( tempIt == rel_map.end() ) { //if relation does not exist
		numRelations++ ;
		relContainer *temp_relContainer = new relContainer(relName, numTuples, numRelations); //allocate memory for new relation container
		//rel_map.insert(temp_relContainer->get_rel_name(), temp_relContainer);
		rel_map[temp_relContainer->get_rel_name()] = temp_relContainer;
		printMsg("Statistics::AddRel:Created new relation....", "");
		printMsg("	Relation:", temp_relContainer->get_rel_name());
		printMsg("	numTuples:", temp_relContainer->get_numTuples());
		printMsg("	partition_num:", temp_relContainer->get_partition_num());
	}

	else { //if relation exists
		relContainer *temp;
		temp = tempIt->second ;
		temp->set_numTuples(numTuples);

		printMsg("Statistics::AddRel:Updated relation....", "");
		printMsg("	Relation:", temp->get_rel_name());
		printMsg("	numTuples:", temp->get_numTuples());
		printMsg("	partition_num:", temp->get_partition_num());
	}

	printMsg("Total number of relations: ", numRelations);
	printMsg("", '\n');
		
}

/* void AddAtt(char*, char*, int)
 * This will add/update the attribute in relName relation. If numDistincts is -1
 * then number of distinct tuples are equal to number of tuples in relation 
 */
void Statistics::AddAtt(char *relName, char *attName, int numDistincts)
{
	//if relation does not exist throw error and exit
	relMap::iterator relMap_Itr;
	relMap_Itr = rel_map.find(relName);

	if( relMap_Itr == rel_map.end() ) {
		cerr<<"Statistics::AddAtt-Relation do not exists!! Exiting..!";
		exit(0);
	}

	else { //else if relation exists
		relContainer *temp_relC = relMap_Itr->second; //get the relation container
		if( numDistincts == -1)
			numDistincts = temp_relC->get_numTuples(); //set number of distinct to number of tuples is the relation

		attMap::iterator attMap_Itr;
		attMap_Itr = temp_relC->att_map.find(attName); //find if attribute exist in relation 
		
		//if attributes do not exist
		if( attMap_Itr == (temp_relC->att_map).end() ) {
			attContainer *tempAtt = new attContainer(attName, numDistincts);
			(temp_relC->att_map)[tempAtt->get_att_name()] = tempAtt; //insert the attribute in relation's map

			int temp_numAtt = temp_relC->get_numAtt();
			temp_numAtt++;
			temp_relC->set_numAtt( temp_numAtt) ; //increment the number of attributes in relation

			printMsg("Statistics::AddAtt:Created new attribute........", "");
			printMsg("	Relation:", (relMap_Itr->second)->get_rel_name());
			printMsg("	att_name:", tempAtt->get_att_name());
			printMsg("	numDistincts:", tempAtt->get_numDistincts());
		}
		//else if attributes exists
		else {
			attContainer *attTemp;
			attTemp = attMap_Itr->second; //get the attribute container
			attTemp->set_numDistincts(numDistincts); //update the distinct number

			printMsg("Statistics::AddAtt:Updated attribute........", "");
			printMsg("	Relation:", (relMap_Itr->second)->get_rel_name());
			printMsg("	att_name:", attTemp->get_att_name());
			printMsg("	numDistincts:", attTemp->get_numDistincts());
		}

		printMsg("Total number of attributes: ", temp_relC->get_numAtt());
		printMsg("", '\n');
	}


}

/* Statistics::CopyRel(char*, char*)
 * This produces a copy of existing oldName relation (i.e. it creates a new relation and then copy the existing one)
 * Then it store the relation under newName
 */
void Statistics::CopyRel(char *oldName, char *newName)
{
	printMsg("Statistics::CopyRel called!", "");

	//Error Checking	
	if( oldName == NULL || newName == NULL || ((string)oldName).empty() || ( (string)oldName).empty() ) {
		cerr<<"Statistics::CopyRel:One of oldName or newName is empty or NULL ! Exiting..."<<endl;
		exit(0);
	}

	relMap::iterator relation_iter;
	relation_iter = rel_map.find(oldName); //find the old relation

	if( relation_iter == rel_map.end() ) { //if relation does not exist
		cerr<<"Statistics::CopyRel - Relation "<<oldName<<" does not exist!! Exiting....";
		exit(0);
	}

	relContainer *oldRelation = relation_iter->second; //get the relation
	relContainer *newRelation = new relContainer( *oldRelation);

	//(*newRelation) = (*oldRelation); //execute the copy constructor
	newRelation->set_rel_name(newName); //change the name

	//change the partition_num of new relation
	numRelations++;
	newRelation->set_partition_num(numRelations);

	rel_map[newRelation->get_rel_name()] = newRelation; //store the relation


	callFunction("Statistics::CopyRel:Created new relation after copying....", (newRelation->print()) );
	
}

/* void Read(char *)
 * This will read the content of statistics on file
 */
void Statistics::Read(char *fromWhere)
{
	printMsg("Statistics::Read - Reading Statistics from file: ", fromWhere);

	if( fromWhere == NULL || ((string)fromWhere).empty() ) {
	}

	FILE *fileObj = fopen(fromWhere, "r");

	if( fileObj == NULL ) {
		//waiting for lixia's response to handle this case
	}

	else { //if fromWhere is not empty

		fread( &numRelations, sizeof(int), 1, fileObj);
		
		for( int j=0; j<numRelations; j++) {
			relContainer *tempRel = new relContainer;
			tempRel->Read(fileObj);
			rel_map[tempRel->get_rel_name()] = tempRel;
		} 



	} 
	//fileObj.close();
	fclose( fileObj );
	callFunction("Statistics::Read - Read from file. The statistics is: ", print() ); 
}

/* void Write(char *)
 * This will write the content of Statistics on file
 */
void Statistics::Write(char *fromWhere)
{
	printMsg("Statistics::Write-Writing Statistics on file...", "");


	FILE *fileObj;
	fileObj = fopen( fromWhere, "w");

	fwrite(&numRelations, sizeof(int), 1, fileObj );
		
	relMap::const_iterator startIt, endIt;
	startIt = rel_map.begin();
	endIt = rel_map.end();

	for( ; startIt != endIt ; ++startIt)
		(startIt->second)->Write(fileObj); 
	
}

/* Apply()
 * This simulates join or selection using Estimate
 */
void  Statistics::Apply(struct AndList *parseTree, char *relNames[], int numToJoin)
{
	printMsg("Statistics::Apply - Applying.................", "");

	if( !checkValidity(relNames, numToJoin) ) 
		exit(0);


	double resultantTuples;
	int tempPartitionNum;
	//if( parseTree != NULL ){
		//change number of tuples
	resultantTuples = Estimate(parseTree, relNames, numToJoin);
	printMsg("Statistics::Apply - resultantTuples: ", resultantTuples);
	
	relMap::iterator temprelIt = rel_map.find(relNames[0]); //get the first relatson
	if( temprelIt == rel_map.end() ) {
		cerr<<"Statistics::Estimate - This relation "<<relNames[0]<<" do not exists!! Exiting..."<<endl;
		exit(0);
	}

	relContainer *tempRelC= temprelIt->second;
	tempPartitionNum = tempRelC->get_partition_num();
	//tempPartitionNum = relNames[0].get_partition_num();
	printMsg("Statistics::Apply - tempPartitionNum: ", tempPartitionNum);

	for( int j=0; j<numToJoin; j++) {

	relMap::iterator relIt = rel_map.find(relNames[j]); //find the relation
	if( relIt == rel_map.end() ) {
			cerr<<"Statistics::Estimate - This relation "<<relNames[j]<<" do not exists!! Exiting..."<<endl;
			exit(0);
		}
		relContainer *tempRel= relIt->second;
		tempRel->set_numTuples(resultantTuples);
		tempRel->set_partition_num(tempPartitionNum);
			//(relNames[j]).set_numTuples(resultantTuples);
			//(relNames[j]).set_partition_num(tempPartitionNum);

	}
		//change number of distincts
		//update partition number
	//}

	printMsg("", '\n');
}

/* It estimate the cost of given query
 * It can handle more than one partition and any number of ORs and ANDs (yeah thats a condescending comment) 
 */
double Statistics::Estimate(struct AndList *parseTree, char **relNames, int numToJoin)
{
	printMsg("Statistics::Estimate - Estimating cost.................", "");
	
	if( !checkValidity(relNames, numToJoin) ) 
		exit(0);

	typedef tr1::unordered_map<int, int> partitionMap; //keep partition number as key and numTuples as value
	double numResult = 1;
	partitionMap partition_map;

	if( parseTree == NULL ) { //do cartesian product of all partitions
		printMsg("Statistics::Estimate - parseTree is NULL ! So performing cartesian join ", "");

		for( int i=0; i<numToJoin; i++) {
			relMap::iterator relIt = rel_map.find(relNames[i]); //find the relation
			if( relIt == rel_map.end() ) {
				cerr<<"Statistics::Estimate - This relation "<<relNames[i]<<" do not exists!! Exiting..."<<endl;
				exit(0);
			}
			relContainer *tempRel= relIt->second;
			partition_map[tempRel->get_partition_num()] = tempRel->get_numTuples();
		}
		//numResult *= tempRel->get_numTuples();

		partitionMap::iterator startIt, endIt;
		startIt = partition_map.begin();
		endIt = partition_map.end();

		for(; startIt != endIt; startIt++)
			numResult *= startIt->second;

		printMsg("Statistics::Estimate - numResult: ", numResult);
		return numResult;
	}


	long int numTuples = 0;
	double denoResult, finalResult;
	denoResult= estimateANDList( parseTree, relNames, numToJoin, numTuples);

	printMsg("Statistics::Estimate - denoResult: ", denoResult );
	
	for( int i=0; i<numToJoin; i++) {
		relMap::iterator relIt = rel_map.find(relNames[i]); //find the relation
		if( relIt == rel_map.end() ) {
			cerr<<"Statistics::Estimate - This relation "<<relNames[i]<<" do not exists!! Exiting..."<<endl;
			exit(0);
		}
		relContainer *tempRel= relIt->second;
		partition_map[tempRel->get_partition_num()] = tempRel->get_numTuples();
		//numResult *= tempRel->get_numTuples();
	}

	partitionMap::iterator startIt, endIt;
	startIt = partition_map.begin();
	endIt = partition_map.end();

	for(; startIt != endIt; startIt++)
		numResult *= startIt->second;
	

	printMsg("Statistics::Estimate - numResult: ", numResult);
	finalResult = (denoResult * numResult );

	printMsg("Statistics::Estimate - finalResult: ", finalResult);
	printMsg("", '\n');
	//print();
	return ( finalResult);
}

/* long int estimateOperand(struct ComparisonOp*)
 * this estimate the cost of a condition e.g (l_item < 100 )
 * Note that it returns only the denominator.
 * Return type is long int because i assume number of distinct tuples of an attribute
 * won't go beyond 2 billion
 */
double Statistics:: estimateOperand(struct ComparisonOp *conditionList, char **relSet, int numJoins, long int &numTuples) {
	printMsg("Statistics:estimateOpreand - Estimating operands....", "");
	//char *leftRelName, *rightRelName;
	string leftAttName, rightAttName, leftRelName, rightRelName;
	int isLeftAttribute=0, isRightAttribute=0;
	long int leftNumDistincts, leftNumTuples, rightNumDistincts, rightNumTuples;

	if( conditionList->left->code == 3 ) { //if left operand is attribute
		isLeftAttribute = 1;

		ParseAttribute( conditionList->left, relSet, numJoins, leftAttName, leftRelName );
		printMsg("estimateOperand - Left Attribute is: ", leftAttName);
		printMsg("estimateOperand - Left Relation is: ", leftRelName );

		//call findAtt
		FindAttData( leftRelName, leftAttName, leftNumDistincts, leftNumTuples );
	}

	if( conditionList->right->code == 3 ) { //if right operand is attribute
		isRightAttribute = 1;

		ParseAttribute( conditionList->right, relSet, numJoins, rightAttName, rightRelName );
		printMsg("estimateOperand - Right Attribute is: ", rightAttName);
		printMsg("estimateOperand - Right Relation is: ", rightRelName );

		//call findAtt
		FindAttData( rightRelName, rightAttName, rightNumDistincts, rightNumTuples );
	}

	//if both operands are NAME
	if( isLeftAttribute == 1 && isRightAttribute == 1 ) {
		//perform join estimation
		//min(leftNumDistincts, rightNumDistincts)
		if( leftNumDistincts > rightNumDistincts )
			return (1.0/((double)leftNumDistincts)); //it could be zero?
		else
			return (1.0/((double)rightNumDistincts)); //it could be zero?
	}
	
	else if( isLeftAttribute == 1 ) {//else perform selection join
		if( conditionList->code == 3 ) { //equality
			numTuples = leftNumTuples;
			return ( 1.0/( (double)leftNumDistincts) ); //estimate for equality is T(R,A)/V(R, A)
		}

		else { //inequality
			numTuples = leftNumTuples;
			return (1.0/3.0 ); //estimate for inequality is T(R,A)/3
		}
	}

	else if( isRightAttribute == 1) {
		if( conditionList->code == 3 ) { //equality
			numTuples = rightNumTuples;
			return (1.0/( (double) rightNumDistincts)); //estimate for equality is T(R,A)/V(R, A)
		}

		else { //inequality
			numTuples = rightNumTuples;
			return (1.0/3.0); //estimate for inequality is T(R,A)/3
		}
	}

	else { //ERROR - both operands are literal. doesn't make senese !
		cerr<<"Statistics:Estimate - both operands are literal !! Exiting..."<<endl;
		exit(0);
	}
}

/* This estimate the cost of OR list 
 * Limitation - Or list is independent only if all conditions are same. Moreover only left attribute is checked
 * i.e. left operand is considered an attribute (I know this is not good. but i am dipped in lazyness right now )
 * Will make it better after submitting project
 */
double Statistics::estimateORList(struct OrList *orList, char **relSet, int numToJoin, long int &numTuples ) {
	printMsg("Statistics::estimateORList - Estimating cost of an ORlist ", "");

	/* Handle the independent case */
	bool isIndependent = true;
	struct OrList *tempOrList = orList;
	struct ComparisonOp *tempCompList = tempOrList->left;
	string value = tempCompList->left->value; //assuming that left has the attribute

	while(tempOrList!= NULL) { //find out if conditions in OR are independent
		if(value.compare(tempOrList->left->left->value) != 0){
			isIndependent = false;
			break;
		}
		tempOrList= tempOrList->rightOr;	
		//tempCompList = tempOrList->left;
	}

	double m;
	if( orList->left != NULL )
		m = estimateOperand(orList->left, relSet, numToJoin, numTuples);

	printMsg("Statistics::estimateORList - Result of first condition: ", m);

	if( orList->rightOr == NULL) //it means there is only one condition
		return (m);
	else { //orList consist of more than one condition so complex formula will rescue us here 
		printMsg("Statistics::estimateORList - It has more than one condition ", "");
		double saveResult = 1;
		if(isIndependent)
			saveResult = m;
		else
			saveResult = (1-m);
		orList = orList->rightOr;	
		while( orList != NULL ) {
			if(orList->left != NULL) {
				m = estimateOperand(orList->left, relSet, numToJoin, numTuples);
				if(isIndependent)
					saveResult += m;
				else
					saveResult *= (1.0-m);
				orList = orList->rightOr; 
			}
			else // that is orList is not NULL but orList->left is NULL !
				break;
		}
		printMsg("Statistics::estimateORlist - Final result of OR list: ", saveResult );
		if(isIndependent)
			return (saveResult);
		else
			return (1.0-saveResult);
	}

}

/* This estimate the cost of an AND list */
double Statistics::estimateANDList(struct AndList *andList, char **relSet, int numToJoin, long int &numTuples ) {
	printMsg("Statistics::estimateAND - Estimating result of AND list ", "");

	double andResult = 1;
	while( andList != NULL) {
		if( andList->left != NULL ) {
			andResult *= estimateORList( andList->left, relSet, numToJoin, numTuples );

			printMsg("Statistics::estimateAND - temp andResult ", andResult ); 
			andList = andList->rightAnd;
		}
		else
			break;
	}

	printMsg("Statistics::estimateAND - Final result of AND list ", andResult ); 
	return andResult;
	
}

void Statistics::ParseAttribute(struct Operand *AnOperand, char **relSet, int numJoin, string &attName, string &relName ) {
	printMsg("Statistics:ParseAttribute- Parsing attributes....", "");
	
	//char *tempRelName;
	string tempAttName, tempRelName;

	int i =0;
	string currentName = AnOperand->value;

	if( (i = currentName.find_first_of( '.' )) !=  string::npos ){ // that is . exists
		tempRelName = currentName.substr(0, i);
		tempAttName = currentName.substr(i+1, (currentName.size() - i) );
	}
	
	else {
		tempAttName = currentName;

		int pos;
		for( pos=0; pos<numJoin; pos++){
			if( hasAttribute( relSet[pos], tempAttName ) )
					break;
		}
		
		if( pos < numJoin ) //found the attribute in relation
			tempRelName = relSet[pos];
		else {
			cerr<<"Statistics::Estimate - Attribute: "<<tempAttName<<" do not exists in any relation ! Exiting.."<<endl;
			exit(0);
		}
		
	}

	attName = tempAttName;
	relName = tempRelName;

}

/* This returns true if relName has attribute attName */
bool Statistics::hasAttribute(string relName, string attName) {
	relMap::iterator relIt = rel_map.find(relName); //find relation
	if( relIt == rel_map.end() ) {
		cerr<<"Statistics::Estimate - This relation "<<relName<<" does not exists ! Exiting ..."<<endl;
		exit(0);
	}

	relContainer *tempRel = relIt->second;
	attMap::iterator attIt = (tempRel->att_map).find(attName); //find the attribute

	if( attIt != (tempRel->att_map).end() )  //it has attribute
		return true;
	else
		return false;

}

/* int FindAttData(string, string, long int&, long int&)
 * this will find the attribute in relation and will return the data
 */
void Statistics::FindAttData(string relName, string attName, long int &numDistincts, long int &numTuples ) {
	printMsg("Statistics:FindAttData- Finding data....", "");
	
	relMap::iterator relIt = rel_map.find(relName);
	if( relIt == rel_map.end() ) {
		cerr<<"Statistics::FindAttData- This relation "<<relName<<" does not exists ! Exiting ..."<<endl;
		exit(0);
	}

	relContainer *tempRel = relIt->second;
	numTuples = tempRel->get_numTuples(); //get the number of tuples in the relation

	attMap::iterator attIt = (tempRel->att_map).find(attName);
	
	if( attIt == (tempRel->att_map).end() ) {
		cerr<<"Statistics::FindAttData- This attribute "<<attName<<" does not exists ! Exiting..."<<endl;
		exit(0);
	}

	attContainer *tempAtt= attIt->second;
	numDistincts = tempAtt->get_numDistincts();

	printMsg("Statistics::FindAttData - numDistincts: ", numDistincts );
	printMsg("Statistics::FindAttData - numTuples: ", numTuples);
}

/* void print()
 * It prints the whole statistics
 */
void Statistics::print() {
	cout<<"Statistics::Printing whole Statistics...."<<endl;
	cout<<"		numRelations:"<<numRelations<<endl;
	
	relMap::const_iterator startIt, endIt;
	startIt = rel_map.begin();
	endIt = rel_map.end();

	for(; startIt != endIt; ++startIt )
		(startIt->second)->print();
	
	cout<<endl;
}

/* This returns true if relSet contains valid partitions */
bool Statistics:: checkValidity(char **relSet, int numToJoin ) { //this check the validity of relSet
	printMsg("Statistics::checkValidity - Checking validity................", "");

	int tempPartNum;
	relMap::iterator startIt, endIt ;
	for( int i=0; i<numToJoin; i++) {
		relMap::iterator relIt = rel_map.find(relSet[i]); //find the relation in rel map
		if( relIt == rel_map.end()){
			cerr<<"Statistics::checkValidity - Relation do not exist !!"<<endl;
			return false;
		}

		relContainer *tempRel = relIt->second;
		tempPartNum =  tempRel->get_partition_num(); //get the partition number
		
		startIt = rel_map.begin();
		endIt = rel_map.end();
		
		//search the map for existing partition number
		for(; startIt != endIt ; startIt++) {
			relContainer *tempRelC = startIt->second;
			if( tempPartNum == tempRelC->get_partition_num() ) {
				string tempRelSet = relSet[i];
				string tempRelInMap = tempRelC->get_rel_name();
				if( tempRelInMap.compare(tempRelSet) == 0 ) //its the same relation we are processing
					continue;

				else {
					int j=0;
					for(j=0; j<numToJoin; j++){ //at this point we have found new relation in same partition//tempRelC should be in relSet
						if( i != j) { //don't compare it with the same relation
							string tempRel = relSet[j]; //get relation from relset
							string relInMap = tempRelC->get_rel_name(); //get relation from relation map
							if ( tempRel.compare(relInMap) == 0 ) //relation exists in relSet
								break; 
						}

					}

					if( j >= numToJoin ) {//it means we didn't find the relation in rel set
						cerr<<"Statistics::checkValidity - Relset have relation which do not exists as singleton anymore!"<<endl;
						return false;
					}
				}
			

			}
		}
	}
	return true;
}
