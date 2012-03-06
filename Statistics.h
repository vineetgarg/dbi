#ifndef STATISTICS_
#define STATISTICS_
#include "ParseTree.h"
#include <string>
#include <tr1/unordered_map>
#include <cstdlib>
#include <fstream>
#include "debugFile.h" //to print debug info

using namespace std;

/* attContainer
 * This class is a container for attributes and maintain all attribute information in the relation
 * @Vineet Garg
 * 4 April, 2011
 */
class attContainer {
	string att_name;
	int numDistincts;

public:
	attContainer();
	attContainer(string att_name, int numDistincts);
	attContainer(const attContainer &copyMe); //copy constructor

	void set_att_name(string att_name);
	string get_att_name();

	void set_numDistincts(int numDistincts);
	int get_numDistincts();
	
	void print(); //it prints the content of attributes - not in requirement but to make my life easy :)
	void Write(FILE *fileObj); //it write the content of attributes to the file
	void Read(FILE *fileObj);

};


typedef std::tr1::unordered_map<string, attContainer*> attMap; //typedef for unordered map of attributes

/* relContainer
 * This class is a container for relations and maintain all attributes in a hash table
 * along with other information 
 * @Vineet Garg
 * 4 April, 2011
 */
class relContainer {
	string rel_name; //relation name
	int numTuples; //total number of tuples
	int partition_num; //partition number it belongs to
	int numAtt; //total attributes in the relation

public:
	attMap att_map; //unordered map to contain attributes
	
	relContainer();
	relContainer(string rel_name, int numTuples, int partition_num); 
	relContainer(const relContainer &copyMe); //copy constructor
	~relContainer();

	void set_rel_name(string rel_name);
	string get_rel_name();
	void set_numTuples(int numTuples);
	int get_numTuples();
	void set_partition_num(int partition_num);
	int get_partition_num();
	void set_numAtt(int numAtt);
	int get_numAtt();

	void print(); //yeah to make my life easy 
	void Write(FILE *fileObj); //no this is the requirement :D
	void Read(FILE *fileObj);

	
};

typedef tr1::unordered_map<string, relContainer*> relMap;

/* Statistics 
 * This class implements Statistics function which in turn is used by Query Optimizer 
 * to optimize queries 
 * @Vineet Garg
 * 4 April, 2011
 */
class Statistics
{
	int numRelations; //total number of relations
	relMap rel_map; // Statistics maintains all the relations in an unordered map to make it easy to access

	double estimateOperand(struct ComparisonOp *conditionList, char **relSet, int numToJoin, long int &numTuples); //this returns the estimated cost of a condition
	double estimateORList(struct OrList *orList, char **relSet, int numToJoin, long int &numTuples );
	double estimateANDList(struct AndList *andList, char **relSet, int numToJoin, long int &numTuples );

	void FindAttData( string relName, string attName, long int &numDistincts, long int &numTuples);
	void ParseAttribute(struct Operand *AnOperand, char **relSet, int numJoin, string &attName, string &relName ) ;
	bool hasAttribute(string relName, string attName);

	bool checkValidity(char **relSet, int numToJoin ); //this check the validity of relSet

public:
	Statistics();
	Statistics(Statistics &copyMe);	 // Performs deep copy
	~Statistics();


	void AddRel(char *relName, int numTuples);
	void AddAtt(char *relName, char *attName, int numDistincts);
	void CopyRel(char *oldName, char *newName);
	
	void Read(char *fromWhere);
	void Write(char *fromWhere);

	void  Apply(struct AndList *parseTree, char *relNames[], int numToJoin);
	double Estimate(struct AndList *parseTree, char **relNames, int numToJoin);
	
	void print(); //yeah you guessed correct. to make life easy
};


#endif
