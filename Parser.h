/* File:   Parser.h
 * This file parse the query then optimize the query. It also execute the query using
 * various operations implemented in initial projects
 * @Vineet Garg
 */

#include "Comparison.h"
#include "Statistics.h"
#include "RelOp.h"
#include "Defs.h"
#include "ParseTree.h"
#include<iostream>
#include "debugFile.h"


#ifndef _PARSER_H
#define	_PARSER_H

#define MAX_ATTS 16

/* this captures the information of all relations involved in a user query */
class TableInfo {
public:
    // this is the original table name
    string tableName;
    // this is the value it is aliased to
    string aliasAs;
    //AndList of a table, it stores all the OrList wherever any parameters from a Table appears
    AndList *andListTable;
    Schema *tableSchema;

    TableInfo(char *tableName, char *aliasAs, Schema *tableSchema) {
        this->tableName = tableName;
        this->aliasAs = aliasAs;
        this->tableSchema = tableSchema;
        andListTable =NULL;
    }
    void setInfo(char *tableName, char *aliasAs, Schema *tableSchema) {
        this->tableName = tableName;
        this->aliasAs = aliasAs;
        this->tableSchema = tableSchema;
        andListTable =NULL;
    }
    TableInfo() {
        tableName = "";
        aliasAs = "";
        tableSchema = NULL;
        andListTable =NULL;
    }
    ~TableInfo();
    AndList *getSelectList();
};

/* This represents a query Node */
class TreeNode {
public:
    relOpMode operType;
    RelOpThreadData *data;
    string tableName;
    Schema* nodeSchema;
    TreeNode *left;
    TreeNode *right;
    int inPipe1;
    int inPipe2;
    int outPipe;

    TreeNode(relOpMode operType, RelOpThreadData *data,string tableName, Schema *nodeSchema,int inPipe1, int inPipe2,int outPipe) {
        this->operType = operType;
        this->data = data;
        this->tableName = tableName;
        this->nodeSchema = nodeSchema;
        this->left =NULL;
        this->right =NULL;
        this->inPipe1 = inPipe1;
        this->inPipe2 = inPipe2;
        this->outPipe = outPipe;
    }
    void Execute();
    void DeleteNode();
    void PrintNode();
};

//This represents a join node */
struct JoinNode {
    AndList *joinAndList;
    TableInfo *table1;
    TableInfo *table2;
    AndList *estimateAndList;

    JoinNode(AndList *joinAnd, AndList *estimateAndList, TableInfo* tbl1, TableInfo * tbl2) {
        this->joinAndList = joinAnd;
        this->estimateAndList = estimateAndList;
        this->table1 = tbl1;
        this->table2 = tbl2;
    }
};
typedef std::tr1::unordered_map<string, TreeNode*> TreeNodeMap;

/* This class parse the query
 * optimzes the query,
 * then it executes the query
 */
class Parser {

    //Information from parser.y
    FuncOperator *finalFunction; // the aggregate function (NULL if no agg)
    TableList *tables; // the list of tables and aliases in the query
    AndList *boolean; // the predicate in the WHERE clause
    NameList *groupingAtts; // grouping atts (NULL if no grouping)
    NameList *attsToSelect; // the set of attributes in the SELECT (NULL if no such atts)
    int distinctAtts; // 1 if there is a DISTINCT in a non-aggregate query
    int distinctFunc;
    
    //store information about Left Deep Join order that we have to implement
    string minJoinCombination;
    double minEstimate;

    //Store information about each Table referred in SQL query
    TableInfo *schemaForAttributesFormation;
    int tableInfoCounter;

    vector<JoinNode *> joinList;
    std::tr1::unordered_map<string, TreeNode*> tableTreeNode;
    
    TreeNode *rootNode;

    // Information for project operation
    int numberOfAttributes;
    int attsToKeep[MAX_ATTS];
    Attribute *projectAtts;
    Schema *tableSchema;
    Schema *projectSchema;
    int PipeIdCount;
public:

    //Constructor And Destructor
    Parser(FuncOperator* finalFunction,TableList* tables,AndList* boolean, NameList* groupingAtts,NameList* attsToSelect,int distinctAtts,int distinctFunc);
    Parser(const Parser& orig);
    Parser();
    ~Parser();
    OrderMaker* getOrderMaker(NameList *groupingAtts,Schema *nodeSchema);
    void QueryParsing();

    void initialize();
    void generateSortArray(int *attsToKeep, int numberOfAttributes, int attNum);
    void getAndListForTable(AndList *inputAnd, TableInfo *table1);
    bool TraversingOrList(OrList *inputOr, TableInfo *table1);
    bool CheckComparisonOp(struct ComparisonOp *inputCom, TableInfo *table1);
    bool checkForAlias(string currentAttribute,string &tableOrAlias, string &modifiedAttribute);
    
    void generateSelectNodes();
    void generateProjectNodes();
    void generateJoinNodes();
    void traverseQueryFunction(FuncOperator *tempFunctionAtts);
    void InsertNode(string, TreeNode *);
    void ExecuteOpAtNode();
    static void PrintOperand(struct Operand *pOperand);
    static void PrintComparisonOp(struct ComparisonOp *pCom);
    static void PrintOrList(struct OrList *pOr);
    static void PrintAndList(AndList *pAnd);

    AndList* getAndListForTable(AndList *inputAnd, TableInfo *table1, TableInfo *table2,bool join);
    int CheckComparisonOp(struct ComparisonOp *inputCom, TableInfo *table1, TableInfo *table2);
    bool TraversingOrList(OrList *inputOr, TableInfo *table1, TableInfo *table2,bool join);
    AndList* getJoinListForJoin(Schema *joinSchema,string primaryTable);
    TableInfo* FetchTableInfo(string primaryTable);
    void removeAliasFromFunction(FuncOperator *tempFunctionAtts);
    bool InsertAttInTable(char *attribute);
    static TableInfo * FindAttInTable(char *attribute, TableInfo *schemaForAttributesFormation, int counter);
    static void InitStatObject(Statistics &statObj);
    static void getNextJoin(AndList *inputAnd, string &table1, string &table2);
    void EnumerateJoin(AndList* inputAnd, TableInfo *schemaForAttributesFormation, int counter);
    bool checkInOrder(char *permuteOrder);
    long joinOrder(char *permuteOrder);
    void findLeftDeepJoinOrder();
    void generatePermutation(int size);
    void DoPermute(char in[],char out[],int used[],int length, int recursLev );
    void PrintSchema(Schema *incomingSchema);
    void PrintTreeNode();
    void doDuplicateRemove();
    void computeSum();
    void ComputeGroupBy();

    void CheckJoinValidity();
    void Execute();
    void insertAtt(int attNum);
    void insertAtt(Schema *schema, char *attribute);
    void generateAtts(Schema *schema);

    //create table functions
    void createTable(TableProperties *tablesProp,int fileType,NameList * attsToSelect);

    //delete table fns
    void deleteTable(TableProperties *tablesProp);

    //insert into table
    void insertIntoTable(TableProperties *tablesProp);
};

#endif	/* _PARSER_H */

