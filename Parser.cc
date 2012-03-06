/* File - Parser.cc
 * To parse, optimize and execute the query
 *  @Vineet Garg
 */

#include "Parser.h"
#include "iostream"
#include "cstdlib"
#include "cstring"
#include <tr1/unordered_map>
#include <typeinfo>
#include "Defs.h"
#include "debugFile.h"

using namespace std;

int RELOP_PAGESIZE = 5;

extern char *catalog_path, *dbfile_dir, *tpch_dir;

//Destruct all the memory allocated to table info */
TableInfo::~TableInfo() {
    printMsg("TableInfo::TableInfo - Destructing............................", "");
    
    //Delete the schema
    delete tableSchema;

    AndList *temp;

    while (andListTable != NULL) {

        temp = andListTable;
        andListTable = andListTable->rightAnd;
        delete temp;
    }
}

/* This will return the table's and list */
AndList* TableInfo::getSelectList() {
    AndList *tempAnd = andListTable;
    bool match;
    AndList *returnAndList = NULL, *returnAndListEnd = NULL;
    while (tempAnd != NULL) {

        OrList *inputOr = tempAnd->left;
        OrList *tempOr = inputOr;
        match = false;
        while (tempOr != NULL) {
            struct ComparisonOp *inputCom = tempOr->left;

            if (inputCom->left->code == NAME) {
                if (inputCom->right->code == NAME) {
                    match = false;
                    break;
                } else if (tableSchema->Find(inputCom->left->value) != -1) {
                    match = true;
                } else {
                    match = false;
                    break;
                }
            }
            tempOr = tempOr->rightOr;
        }
        if (match == true) {
            AndList *temp1 = new AndList;
            temp1->rightAnd = NULL;
            temp1->left = inputOr;
            //copy the inputOr into temp
            if (returnAndList != NULL) {
                returnAndListEnd->rightAnd = temp1;
                returnAndListEnd = temp1;
            } else {
                returnAndList = temp1;
                returnAndListEnd = returnAndList;
            }
        }
        tempAnd = tempAnd->rightAnd;
    }
    return returnAndList;
}

//Explicitly delete each and every node
void TreeNode::DeleteNode() {
    printMsg("TreeNode::DeleteNode  - Deleting each and every node of logical query plan ", "");
    {
        if (right != NULL) {
            right->DeleteNode();
            delete right;
            right = NULL;
        }
        if (left != NULL) {
            left->DeleteNode();
            delete left;
            right = NULL;
        }
    }
    Pipe *output;
    CNF *cnf;
    Record *literal;
    Function *computeMe;
    DBFile *dbFile;
    int *keepMe, *tempNumAttsInput, *tempNumAttsOutput;

    OrderMaker *groupAtts;

    switch (operType) {
        case SELECT_FILE:
 	    printMsg("TreeNode::DeleteNode - Deletingn SELECT_FILE node", "");
            dbFile = (DBFile*) data->getAtt(1);
            output = (Pipe*) data->getAtt(2);
            cnf = (CNF*) data->getAtt(3);
            literal = (Record*) data->getAtt(4);

            delete cnf;
            delete literal;
            dbFile->Close();
            delete dbFile;
            delete output;
            delete data;
            break;
        case PROJECT:
 	    printMsg("TreeNode::DeleteNode - Deletingn PROJECT node", "");
            output = (Pipe*) data->getAtt(2);
            tempNumAttsInput = (int*) data->getAtt(4);
            tempNumAttsOutput = (int*) data->getAtt(5);

            delete output;
            //delete []keepMe; //Delete it in parser Destructor directly
            delete tempNumAttsInput;
            delete tempNumAttsOutput;
            delete data;
            break;
        case JOIN:
 	    printMsg("TreeNode::DeleteNode - Deleting JOIN node", "");
            output = (Pipe*) data->getAtt(3);
            cnf = (CNF*) data->getAtt(4);
            literal = (Record*) data->getAtt(5);

            delete output;
            delete cnf;
            delete literal;
            delete nodeSchema;
            delete data;
            break;
        case DUPLICATE_REMOVAL:
 	    printMsg("TreeNode::DeleteNode - Deleting DISTINCT node", "");
            output = (Pipe *) data->getAtt(2);

            delete output;
            delete data;
            break;
        case SUM:
 	    printMsg("TreeNode::DeleteNode - Deleting SUM node", "");
            output = (Pipe*) data->getAtt(2);
            computeMe = (Function *) data->getAtt(3);

            delete nodeSchema;
            delete output;
            delete computeMe;
            delete data;
            break;
        case GROUPBY:

 	    printMsg("TreeNode::DeleteNode - Deleting GROUPBY node", "");

            output = (Pipe*) data->getAtt(2);
            groupAtts = (OrderMaker *) data->getAtt(3);
            computeMe = (Function *) data->getAtt(4);

            delete output;
            delete groupAtts;
            delete computeMe;
            delete data;
            break;
        case WRITEOUT:

 	    printMsg("TreeNode::DeleteNode - WRITEOUT node", "");
            break;
    }
}

/* Tree Node Function Declaration */
void TreeNode::PrintNode() {
 
    //common attributes that will be used for each type of operation
    CNF *cnf;
    int *keepMe, *tempNumAttsOutput;
    Function *computeMe;
    OrderMaker *groupAtts;
    int count = 0;

    //execute operation on left and right child
    {
        if (right != NULL) {
            right->PrintNode();
        }
        if (left != NULL) {
            left->PrintNode();
        }
    }
    switch (operType) {
        case SELECT_FILE:
            cout << "Operation:SELECT FILE" << endl;
            cout << "Output Pipe:" << outPipe << endl;
            cnf = (CNF*) data->getAtt(3);
            cout << "Select CNF: ";
            cnf->Print();
	    cout<<endl;

            break;
        case PROJECT:
            keepMe = (int*) data->getAtt(3);
            tempNumAttsOutput = (int*) data->getAtt(5);
            cout << "Operation: PROJECT" << endl;
            cout << "Input Pipe:" << inPipe1 << endl;
            cout << "Output Pipe:" << outPipe << endl;

            cout << "Atts to Keep is:";
            for (count = 0; count < *tempNumAttsOutput; count++) {
                cout << keepMe[count] << " ";
            }
            cout << endl;
            break;
        case JOIN:
            cout << "Operation: Join" << endl;
            cout << "Input Pipe:" << inPipe1 << endl;
            cout << "Input Pipe:" << inPipe2 << endl;
            cout << "Output Pipe:" << outPipe << endl;
            cnf = (CNF*) data->getAtt(4);
            cout << "Join CNF: ";
            cnf->Print();
	    cout<<endl;
            break;
        case DUPLICATE_REMOVAL:
            cout << "Operation: DISTINCT" << endl;
            cout << "Input Pipe:" << inPipe1 << endl;
            cout << "Output Pipe:" << outPipe << endl;
	    cout<<endl;
            break;
        case SUM:
            computeMe = (Function *) data->getAtt(3);
            cout << "Operation: SUM" << endl;
            cout << "Input Pipe:" << inPipe1 << endl;
            cout << "Output Pipe:" << outPipe << endl;
            cout << "Compute function for Sum: ";
            computeMe->Print();
	    cout<<endl;
            break;
        case GROUPBY:
            computeMe = (Function *) data->getAtt(4);
            groupAtts = (OrderMaker *) data->getAtt(3);
            cout << "Operation: GROUP BY" << endl;
            cout << "Input Pipe:" << inPipe1 << endl;
            cout << "Output Pipe:" << outPipe << endl;
            cout << "Compute function for GroupBy: ";
            computeMe->Print();
            cout << "Grouping attributes for GroupyBy:\n";
            groupAtts->Print();
	    cout<<endl;
            break;
        default:
            cout << "Wrong OPeration type\n...Exiting the system";
            exit(1);
    }
}

void TreeNode::Execute() {

    //execute operation on left and right child
    {
        if (right != NULL) {
            right->Execute();
        }
        if (left != NULL) {
            left->Execute();
        }
    }
    //common attributes that will be used for each type of operation
    RelationalOp *nodeOp; // = node->relOper;
    Pipe *inPipe;
    Pipe *output;
    CNF *cnf;
    Record *literal;
    Function *computeMe;
    Schema *schema;

    //for SELECT FILE
    SelectFile *select;
    DBFile *dbFile;

    //for JOIN
    Join *join;
    Pipe *inPipeL;
    Pipe *inPipeR;

    //for SUM
    Sum *sum;

    //for DUPLICATE REMOVAL
    DuplicateRemoval *duplicateRemoval;

    //for Project
    Project *project;
    int *keepMe, *tempNumAttsInput, *tempNumAttsOutput;

    //for GROUPBY
    GroupBy *groupby;
    OrderMaker *groupAtts;

    switch (operType) {
        case SELECT_FILE:
            nodeOp = new SelectFile();
	    printMsg("TreeNode::Execute - Executing SELECT_FILE operation....", "");
            select = (SelectFile*) nodeOp;
            dbFile = (DBFile*) data->getAtt(1);
            output = (Pipe*) data->getAtt(2);
            cnf = (CNF*) data->getAtt(3);
            literal = (Record*) data->getAtt(4);

            //Now execute the run operation
            select->Run(*dbFile, *output, *cnf, *literal);
            break;
        case SELECT_PIPE:
	    printMsg("TreeNode::Execute - Executing SELECT_PIPE operation....", "");
            break;
        case PROJECT:
            nodeOp = new Project();

	    printMsg("TreeNode::Execute - Executing PROJECT operation....", "");
            project = (Project *) nodeOp;
            inPipe = (Pipe *) data->getAtt(1);
            output = (Pipe*) data->getAtt(2);
            keepMe = (int*) data->getAtt(3);
            tempNumAttsInput = (int*) data->getAtt(4);
            tempNumAttsOutput = (int*) data->getAtt(5);

            project->Run(*inPipe, *output, keepMe, *tempNumAttsInput, *tempNumAttsOutput);
            break;
        case JOIN:
            nodeOp = new Join();

	    printMsg("TreeNode::Execute - Executing JOIN operation....", "");
            join = (Join *) nodeOp;
            inPipeL = (Pipe*) data->getAtt(1);
            inPipeR = (Pipe*) data->getAtt(2);
            output = (Pipe*) data->getAtt(3);
            cnf = (CNF*) data->getAtt(4);
            literal = (Record*) data->getAtt(5);

            join->Run(*inPipeL, *inPipeR, *output, *cnf, *literal);
            break;
        case DUPLICATE_REMOVAL:

	    printMsg("TreeNode::Execute - Executing DISTINCT operation....", "");
            nodeOp = new DuplicateRemoval();
            duplicateRemoval = (DuplicateRemoval *) nodeOp;
            inPipe = (Pipe *) data->getAtt(1);
            output = (Pipe *) data->getAtt(2);
            schema = (Schema *) data->getAtt(3);
            duplicateRemoval->Run(*inPipe, *output, *schema);
            break;
        case SUM:

	    printMsg("TreeNode::Execute - Executing SUM operation....", "");
            nodeOp = new Sum();
            sum = (Sum *) nodeOp;
            inPipe = (Pipe*) data->getAtt(1);
            output = (Pipe*) data->getAtt(2);
            computeMe = (Function *) data->getAtt(3);

            sum->Run(*inPipe, *output, *computeMe);
            break;
        case GROUPBY:

	    printMsg("TreeNode::Execute - Executing GROUPBY operation....", "");
            nodeOp = new GroupBy();
            groupby = (GroupBy *) nodeOp;
            inPipe = (Pipe*) data->getAtt(1);
            output = (Pipe*) data->getAtt(2);
            groupAtts = (OrderMaker *) data->getAtt(3);
            computeMe = (Function *) data->getAtt(4);
	    
	    printMsg("groupby: ", groupby);
	    printMsg("inPipe: ", inPipe);
	    printMsg("output: ", output);
	    printMsg("groupAtts: ", groupAtts);
	    printMsg("computeMe: ", computeMe);

            groupby->Run(*inPipe, *output, *groupAtts, *computeMe);
            break;
        case WRITEOUT:

	    printMsg("TreeNode::Execute - Executing WRITEOUT operation....", "");
            break;
    }
}

/*            Parser Member Functions Declaration  */
Parser::Parser(FuncOperator* finalFunction, TableList* tables, AndList* boolean, NameList* groupingAtts, NameList* attsToSelect, int distinctAtts, int distinctFunc) {
    this->finalFunction = finalFunction;
    this->tables = tables;
    this->boolean = boolean;
    this->groupingAtts = groupingAtts;
    this->attsToSelect = attsToSelect;
    this->distinctAtts = distinctAtts;
    this->distinctFunc = distinctFunc;
    this->numberOfAttributes = 0;
}

Parser::Parser() {
    numberOfAttributes = 0;
}

Parser::Parser(const Parser& orig) {
    numberOfAttributes = 0;
}

Parser::~Parser() {

    //Free Up memory for Table Info
    if(boolean !=NULL){
        delete []schemaForAttributesFormation;

        TreeNodeMap::iterator mapIt = tableTreeNode.begin();
        TreeNode *node = mapIt->second;

        node->DeleteNode();
        delete node;
    }
    
}

void Parser::QueryParsing() {
	printMsg("Parser::QueryParsing - Parsing query...", "");

    PipeIdCount = 1;
    time_t start, end;
    time(&start);
    initialize();

    //Parse and push down selections
    generateSelectNodes();

   //Find optimized join order 
    findLeftDeepJoinOrder();

   //parse and optimize join 
    generateJoinNodes();

    
    CheckJoinValidity(); //to verify the generation of join nodes

    if (groupingAtts != NULL) 
        ComputeGroupBy(); //optimize group by

    else if (finalFunction != NULL) 
        computeSum(); //compute sum

    else {
        generateProjectNodes(); //projection

        if (distinctAtts == 1) {
            doDuplicateRemove(); //distinct
        }
    }

    Execute();

    TreeNodeMap::iterator mapIt = tableTreeNode.begin();
    TreeNode *node = mapIt->second;
    Record tempRecord;
    Pipe *output;
    if (node->operType == JOIN) {
        output = (Pipe *) node->data->getAtt(3);
    } else {
        output = (Pipe *) node->data->getAtt(2);
    }


    //Display the output
    int recordCounter = 0;
    cout << "The query returned following results :\n\n";
    
    while (output->Remove(&tempRecord)) {

        recordCounter++;

        tempRecord.Print(node->nodeSchema);
    }

    cout << "\nTotal Records Fetched: " << recordCounter << endl;
    time(&end);
    double diff = difftime(end, start);
    cout<<"\nTotal time taken to parse, optimze and execute the query: "<<diff<<endl<<endl;
}

/* Initialize the Information required for parsing */
void Parser::initialize() {
	printMsg("Parser::initializing - Initialzing parser...............", "");

    TableList *tempTablePtr;
    tempTablePtr = tables;
    tableInfoCounter = 0;

    //find the number of tables and store it in tableInfoCounter
    while (tempTablePtr != NULL) {
        tableInfoCounter++;
        tempTablePtr = tempTablePtr->next;
    }
    //Allocate the required memory
    schemaForAttributesFormation = new TableInfo[tableInfoCounter];

    int tableCounter = 0;
    tempTablePtr = tables;
    while (tempTablePtr != NULL) {
        TableInfo *temp = &schemaForAttributesFormation[tableCounter];
        temp->setInfo(tempTablePtr->tableName, tempTablePtr->aliasAs, new Schema(catalog_path, tempTablePtr->tableName));
        //Generate the AndList correspond to each table
        getAndListForTable(boolean, temp);
        tableCounter++;
	callFunction("Full AndList : ", PrintAndList(temp->andListTable) );
        tempTablePtr = tempTablePtr->next;
    }
}

/* it parses the where's and list and try to push down as many selections as it can */
void Parser::generateSelectNodes() {
	printMsg("Parser::generateSelectNodes - Generating select nodes for all relations......", "");
    for (int count = 0; count < tableInfoCounter; count++) {
        //Find the AndList correspond to a particular table
        TableInfo *temp = &schemaForAttributesFormation[count];

        AndList *tableAndList = temp->getSelectList();

        //AndList *tableAndList = getAndListForTable(boolean,temp,NULL,false);
	printMsg("Parser::generateSelectNodes - And List for table: ", temp->tableName );
	callFunction("", PrintAndList(tableAndList) );

        //generate CNF from AndList
        CNF *cnf = new CNF;
        Record *literal = new Record;
        // constructs CNF predicate
        cnf->GrowFromParseTree(tableAndList, temp->tableSchema, *literal);

        if (PRINT_LEVEL == DEBUG) {
            cout << "CNF is as follows\n";
            cnf->Print();
        }

        //Create the thread data structure for SelectFile
        RelOpThreadData *data = new RelOpThreadData(4);

        string tablePath = dbfile_dir + schemaForAttributesFormation[count].tableName + ".bin";
        DBFile *inFile = new DBFile();

        inFile->Open((char*) tablePath.c_str());
        inFile->MoveFirst();
        data->setAtt(1, inFile);

        Pipe *outPipe = new Pipe(PIPE_SIZE);
        data->setAtt(2, outPipe);
        data->setAtt(3, cnf);
        data->setAtt(4, literal);

        TreeNode *node = new TreeNode(SELECT_FILE, data, temp->tableName, temp->tableSchema, 0, 0, PipeIdCount++);

        node->left = NULL;
        node->right = NULL;
        this->InsertNode(temp->tableName, node);
    }
}

void Parser::getAndListForTable(AndList* inputAnd, TableInfo* table1) {

    AndList *temp = NULL;
    AndList *andListEnd = NULL;
    bool result = false;
    while (inputAnd != NULL) {

        OrList *inputOr = inputAnd->left;
        result = TraversingOrList(inputOr, table1);

        if (result) {
            AndList *temp1 = new AndList;
            temp1->rightAnd = NULL;
            temp1->left = inputOr;

            //copy the inputOr into temp
            if (temp != NULL) {
                //append at the end
                andListEnd->rightAnd = temp1;
                andListEnd = temp1;
            } else {
                temp = temp1;
                andListEnd = temp;
            }
        }
        //increment the inputAnd
        inputAnd = inputAnd->rightAnd;
    }
    //Store the AndList in the TableInfo* table1 passed in the parameter
    table1->andListTable = temp;
}

bool Parser::TraversingOrList(OrList *inputOr, TableInfo *table1) {

    int returnValue = 0;
    bool match = false;
    while (inputOr != NULL) {
        struct ComparisonOp *inputCom = inputOr->left;
        returnValue = CheckComparisonOp(inputCom, table1);

        //if any found return TRUE to save it in AndList of table
        if (match == false)
            match = returnValue;

        inputOr = inputOr->rightOr;
    }
    return match;
}

bool Parser::CheckComparisonOp(struct ComparisonOp *inputCom, TableInfo *table1) {

    bool match = false;
    string tableOrAlias;
    string modifiedAttribute;

    if (inputCom != NULL) {
        if (inputCom->left->code == NAME) {
            if (checkForAlias(inputCom->left->value, tableOrAlias, modifiedAttribute)) {

                //modify the parameters so that we don't have to check for alias again
                strcpy(inputCom->left->value, modifiedAttribute.c_str());
                if ((table1->tableName == tableOrAlias || table1->aliasAs == tableOrAlias)) {
                    if (table1->tableSchema->Find((char*) modifiedAttribute.c_str()) != -1) {
                        if (match == false)
                            match = true;
                    } else {
                        cout << "Unable to find attribute " << modifiedAttribute << " in table" << tableOrAlias << endl;
                        cout << "Exiting the system" << endl;
                        exit(1);
                    }
                }
            } else if (table1->tableSchema->Find(inputCom->left->value) != -1) {
                if (match == false)
                    match = true;
            }
        }
        //Although we don't need to check right parameter, check it to modify the parameter
        if (inputCom->right->code == NAME) {

            if (checkForAlias(inputCom->right->value, tableOrAlias, modifiedAttribute)) {
                strcpy(inputCom->right->value, modifiedAttribute.c_str());
                if ((table1->tableName == tableOrAlias || table1->aliasAs == tableOrAlias)) {
                    if (table1->tableSchema->Find((char*) modifiedAttribute.c_str()) != -1) {
                        if (match == false)
                            match = true;
                    } else {
                        cout << "Unable to find attribute " << modifiedAttribute << " in table" << tableOrAlias << endl;
                        cout << "Exiting the system" << endl;
                        exit(1);
                    }
                }
            } else if (table1->tableSchema->Find(inputCom->right->value) != -1) {
                if (match == false)
                    match = true;
            }
        }
    }
    return match;
}

bool Parser::checkForAlias(string currentAttribute, string &tableOrAlias, string &modifiedAttribute) {

    int found = currentAttribute.find(".");

    if (found != string::npos) {

        //split the string into tableName or alias and attribute
        tableOrAlias = currentAttribute.substr(0, found);
        modifiedAttribute = currentAttribute.substr(found + 1);
        return true;
    } else
        return false;
}

void Parser::doDuplicateRemove() {
    TreeNodeMap::iterator mapIt = tableTreeNode.begin();

    if (mapIt == tableTreeNode.end()) {
        cout << "Not able to find the input pipe for duplicate removal.Something went wrong....\n";
        cout << "Exiting the system\n";
        exit(1);
    }
    TreeNode *currentRootNode = mapIt->second;
    RelOpThreadData *data = new RelOpThreadData(3);

    data->setAtt(1, currentRootNode->data->getAtt(2));
    Pipe *outPipe = new Pipe(PIPE_SIZE);
    data->setAtt(2, outPipe);
    data->setAtt(3, currentRootNode->nodeSchema);

    TreeNode *rootNode = new TreeNode(DUPLICATE_REMOVAL, data, currentRootNode->tableName, currentRootNode->nodeSchema, currentRootNode->outPipe, 0, PipeIdCount++);

    tableTreeNode.erase(mapIt);
    rootNode->left = currentRootNode;
    rootNode->right = NULL;

    this->InsertNode(currentRootNode->tableName, rootNode);
}

void Parser::computeSum() {

    TreeNodeMap::iterator mapIt = tableTreeNode.begin();

    if (mapIt == tableTreeNode.end()) {
        cout << "Not able to find the input pipe for sum removal.Something went wrong....\n";
        cout << "Exiting the system\n";
        exit(1);
    }
    TreeNode *currentRootNode = mapIt->second;

    RelOpThreadData *data = new RelOpThreadData(3);

    if (currentRootNode->operType == JOIN) {
        data->setAtt(1, currentRootNode->data->getAtt(3));
    } else {
        data->setAtt(1, currentRootNode->data->getAtt(2));
    }
    //data->setAtt(1,currentRootNode->data->getAtt(2));
    Pipe *outPipe = new Pipe(PIPE_SIZE);
    data->setAtt(2, outPipe);

    //traverse function first to remove Alias from it
    removeAliasFromFunction(finalFunction);
    Function *funcPred = new Function;
    funcPred->GrowFromParseTree(finalFunction, *currentRootNode->nodeSchema);
    data->setAtt(3, funcPred);

    Attribute *DA = new Attribute();
    DA->name = (char *) "Sum";
    DA->myType = Double;

    Schema *out_sch = new Schema((char *) "out_sch", 1, DA);
    rootNode = new TreeNode(SUM, data, currentRootNode->tableName, out_sch, currentRootNode->outPipe, 0, PipeIdCount++);

    tableTreeNode.clear();
    rootNode->left = currentRootNode;
    rootNode->right = NULL;

    this->InsertNode(currentRootNode->tableName, rootNode);
}

void Parser::removeAliasFromFunction(FuncOperator* tempFunctionAtts) {

    if (tempFunctionAtts == NULL)
        return;

    string tableOrAlias;
    string modifiedAttribute;
    if (tempFunctionAtts->leftOperand != 0) {

        if (tempFunctionAtts->leftOperand->code == NAME) {
            if (checkForAlias(tempFunctionAtts->leftOperand->value, tableOrAlias, modifiedAttribute)) {
                strcpy(tempFunctionAtts->leftOperand->value, modifiedAttribute.c_str());
            }
        }
    }
    if (tempFunctionAtts->leftOperator != 0) {
        removeAliasFromFunction(tempFunctionAtts->leftOperator);
    }
    if (tempFunctionAtts->right != 0) {
        removeAliasFromFunction(tempFunctionAtts->right);
    }
}

OrderMaker* Parser::getOrderMaker(NameList *groupingAtts, Schema *nodeSchema) {

    NameList *tempGroupAtts;
    //Traverse GroupBy list
    tempGroupAtts = groupingAtts;

    AndList *final = NULL;
    AndList *endFinalList;

    string tableOrAlias;
    string modifiedAttribute;

    while (tempGroupAtts != NULL) {

        if (checkForAlias(tempGroupAtts->name, tableOrAlias, modifiedAttribute)) {
            strcpy(tempGroupAtts->name, modifiedAttribute.c_str());
        }

        Operand *op = new Operand;
        op->code = NAME;
        op->value = tempGroupAtts->name;

	printMsg("Parser::getOrderMaker - Group by atts: ", tempGroupAtts->name );

        ComparisonOp *compOp = new ComparisonOp;
        compOp->left = op;
        compOp->right = op;
        compOp->code = EQUALS;

        OrList *orList = new OrList;
        orList->left = compOp;
        orList->rightOr = NULL;

        AndList *tempAndList = new AndList;
        tempAndList->left = orList;
        tempAndList->rightAnd = NULL;

        if (final == NULL) {
            final = tempAndList;
            endFinalList = final;
        } else {
            endFinalList->rightAnd = tempAndList;
            endFinalList = tempAndList;
        }
        tempGroupAtts = tempGroupAtts->next;
    }

    callFunction("Parser::getOrderMaker - And list for group by: ", PrintAndList(final) );

    callFunction("Parser::getOrderMaker - Group Schema: ", PrintSchema(nodeSchema) );

    CNF *cnf_grp = new CNF;
    Record *dummy = new Record;

    cnf_grp->GrowFromParseTree(final, nodeSchema, *dummy); // constructs CNF predicate
    OrderMaker *grp_order = new OrderMaker;
    OrderMaker dummyOM;
    cnf_grp->GetSortOrders(*grp_order, dummyOM);
    if (PRINT_LEVEL == DEBUG) {
        cout << "cnf_grp->Print();" << endl;
        cnf_grp->Print();
    }
    //delete memory allocated for AndList final
    AndList *temp;
    while (final != NULL) {
        temp = final;

        final = final->rightAnd;

        OrList *orList = temp->left;
        ComparisonOp *compOp = orList->left;
        Operand *op = compOp->left;

        delete op;
        delete compOp;
        delete orList;
        delete temp;
    }
    return grp_order;
}

void Parser::ComputeGroupBy() {

    TreeNodeMap::iterator mapIt = tableTreeNode.begin();

    if (mapIt == tableTreeNode.end()) {
        cout << "Not able to find the input pipe for sum removal.Something went wrong....\n";
        cout << "Exiting the system\n";
        exit(1);
    }
    TreeNode *currentRootNode = mapIt->second;
    Schema *currentNodeSchema = currentRootNode->nodeSchema;
    Attribute *currentNodeAtts = currentRootNode->nodeSchema->GetAtts();
    int countGroupAtts = 0;
    int numGroupAtts = 0;
    NameList *tempGroupAtts = groupingAtts;
    while (tempGroupAtts != NULL) {
        numGroupAtts++;
        tempGroupAtts = tempGroupAtts->next;
    }
    Attribute *groupAtts = new Attribute[numGroupAtts + 1];
    int *groupAttsLocation = new int[numGroupAtts];
    groupAtts[countGroupAtts].myType = Double;
    groupAtts[countGroupAtts].name = strdup("Sum");
    countGroupAtts++;

    //Traverse GroupBy list
    tempGroupAtts = groupingAtts;

    AndList *final = NULL;
    AndList *endFinalList;

    string tableOrAlias;
    string modifiedAttribute;
    while (tempGroupAtts != NULL) {

        if (checkForAlias(tempGroupAtts->name, tableOrAlias, modifiedAttribute)) {
            strcpy(tempGroupAtts->name, modifiedAttribute.c_str());
        }

        //search for name to generate schema
        int location = currentNodeSchema->Find(tempGroupAtts->name);

        if (location == -1) {
            cout << "Attribute " << tempGroupAtts->name << " is not found in current node schema. Some error occcured. Exiting the sytem";
            exit(1);
        }
        generateSortArray(groupAttsLocation, countGroupAtts, location);
        countGroupAtts++;

        Operand *op = new Operand;
        op->code = NAME;
        op->value = tempGroupAtts->name;

	printMsg("Parser::computeGroupBy - Group by atts - ", tempGroupAtts->name );

        ComparisonOp *compOp = new ComparisonOp;
        compOp->left = op;
        compOp->right = op;
        compOp->code = EQUALS;

        OrList *orList = new OrList;
        orList->left = compOp;
        orList->rightOr = NULL;

        AndList *tempAndList = new AndList;
        tempAndList->left = orList;
        tempAndList->rightAnd = NULL;

        if (final == NULL) {
            final = tempAndList;
            endFinalList = final;
        } else {
            endFinalList->rightAnd = tempAndList;
            endFinalList = tempAndList;
        }
        tempGroupAtts = tempGroupAtts->next;
    }

    for (int count = 0; count < numGroupAtts; count++) {
        groupAtts[count + 1].myType = currentNodeAtts[groupAttsLocation[count]].myType;
        groupAtts[count + 1].name = strdup(currentNodeAtts[groupAttsLocation[count]].name);
    }

    callFunction("Parser::computeGroupBy - And list for group by: ", PrintAndList(final) );

    Schema *nodeSchema = new Schema((char*) "GroupBy", countGroupAtts, groupAtts);

    callFunction("Parser::computeGroupBy - Group Schema: ", PrintSchema(nodeSchema) );

    CNF *cnf_grp = new CNF;
    Record *dummy = new Record;
    Function *funcPred = new Function;

    removeAliasFromFunction(finalFunction);
    cnf_grp->GrowFromParseTree(final, currentRootNode->nodeSchema, *dummy); // constructs CNF predicate
    funcPred->GrowFromParseTree(finalFunction, *currentRootNode->nodeSchema);
    RelOpThreadData *data = new RelOpThreadData(4);

    if (currentRootNode->operType == JOIN) {
        data->setAtt(1, currentRootNode->data->getAtt(3));
    } else {
        data->setAtt(1, currentRootNode->data->getAtt(2));
    }

   printMsg("Parser::computeGroupBy - cnf: ", cnf_grp->Print() );
    OrderMaker *grp_order = new OrderMaker;
    OrderMaker dummyOM;
    cnf_grp->GetSortOrders(*grp_order, dummyOM);

   printMsg("Parser::computeGroupBy - Order maker for group by: ", grp_order->Print() );

    Pipe *outPipe = new Pipe(PIPE_SIZE);
    data->setAtt(2, outPipe);
    data->setAtt(3, grp_order);
    data->setAtt(4, funcPred);

    TreeNode *rootNode = new TreeNode(GROUPBY, data, currentRootNode->tableName, nodeSchema, currentRootNode->outPipe, 0, PipeIdCount++);
    tableTreeNode.clear();
    rootNode->left = currentRootNode;
    rootNode->right = NULL;

    this->InsertNode(currentRootNode->tableName, rootNode);

    //delete memory allocated for AndList final
    AndList *temp;
    while (final != NULL) {
        temp = final;

        final = final->rightAnd;

        OrList *orList = temp->left;
        ComparisonOp *compOp = orList->left;
        Operand *op = compOp->left;

        delete op;
        delete compOp;
        delete orList;
        delete temp;
    }
    delete []groupAtts;
    delete []groupAttsLocation;
}

void Parser::generateProjectNodes() {

    TreeNodeMap::iterator mapIt;
    mapIt = tableTreeNode.begin();

    TreeNode *currentNode;
    if (mapIt != tableTreeNode.end())
        currentNode = mapIt->second;
    else {
        cout << "No node found in the Tree to do project operation" << endl;
        cout << "Exiting the system\n";
        exit(1);
    }

    //generate attributes list for project on each table by traversing various attributes list
    if (PRINT_LEVEL == DEBUG) {
        PrintSchema(currentNode->nodeSchema);
        cout << "Generate attribute now\n";
    }

    //Traverse Select list
    NameList *tempAttSelect = attsToSelect;
    string tableOrAlias;
    string modifiedAttribute;
    while (tempAttSelect != NULL) {
        if (checkForAlias(tempAttSelect->name, tableOrAlias, modifiedAttribute)) {
            strcpy(tempAttSelect->name, modifiedAttribute.c_str());
        }
        insertAtt(currentNode->nodeSchema, tempAttSelect->name);
        tempAttSelect = tempAttSelect->next;
    }
    //Now generate the project nodes on top of root node using current Node schema
    generateAtts(currentNode->nodeSchema);

    if (PRINT_LEVEL == DEBUG) {
        cout << "Current node" << currentNode->tableName << " " << currentNode->operType << endl;
        PrintSchema(projectSchema);
    }
    RelOpThreadData *data = new RelOpThreadData(5);

    int *schemaNumAtts = new int;
    int *numAttsOuput = new int;

    *schemaNumAtts = currentNode->nodeSchema->GetNumAtts();
    *numAttsOuput = projectSchema->GetNumAtts();

    if (PRINT_LEVEL == DEBUG)
        cout << attsToKeep[0] << " " << *schemaNumAtts << " " << *numAttsOuput << endl;

    // Select output pipe will be input pipe for Project
    if (currentNode->operType == JOIN) {
        data->setAtt(1, currentNode->data->getAtt(3));
    } else {
        data->setAtt(1, currentNode->data->getAtt(2));
    }

    Pipe *outPipe = new Pipe(PIPE_SIZE);
    data->setAtt(2, outPipe);
    data->setAtt(3, (int*) attsToKeep);
    data->setAtt(4, (int*) schemaNumAtts);
    data->setAtt(5, (int*) numAttsOuput);

    //generate the project node for a particular table
    TreeNode *projectNode = new TreeNode(PROJECT, data, currentNode->tableName, projectSchema, currentNode->outPipe, 0, PipeIdCount++);

    tableTreeNode.clear();
    projectNode->left = currentNode;
    projectNode->right = NULL;
    this->InsertNode(currentNode->tableName, projectNode);
}

void Parser::generateJoinNodes() {

    std::tr1::unordered_map<string, int> forRelNameFormation;
    std::tr1::unordered_map<string, int>::iterator forRelNameFormationIterator;
    TreeNodeMap::iterator mapIt;
    //AndList *currentJoin;
    AndList *currentJoinList;
    if (PRINT_LEVEL == DEBUG)
        cout << "Join combination is as follows:" << minJoinCombination.c_str() << endl;
    for (int count = 0; count < joinList.size(); count++) {
        int currentOrder = minJoinCombination[count] - 97;

        if (PRINT_LEVEL == DEBUG)
            cout << "Current join is as follows: " << currentOrder << endl;
        JoinNode *currentJoinNode = joinList[currentOrder];
        TreeNode *left;
        TreeNode *right;

        mapIt = tableTreeNode.find(currentJoinNode->table1->tableName);

        if (mapIt != tableTreeNode.end()) {
            if (PRINT_LEVEL == DEBUG)
                cout << "Left table is: " << mapIt->first << endl;
            left = mapIt->second;
        } else {
            cout << "Select node corresponds to table " << currentJoinNode->table1->tableName << " not found\n";
            exit(1);
        }
        mapIt = tableTreeNode.find(currentJoinNode->table2->tableName);

        if (mapIt != tableTreeNode.end()) {
            if (PRINT_LEVEL == DEBUG)
                cout << "Right table is: " << mapIt->first << endl;
            right = mapIt->second;
        } else {
            cout << "Select node corresponds to table " << currentJoinNode->table2->tableName << " not found\n";
            exit(1);
        }
        //now generate the schema for new join
        int leftAttributesSize = left->nodeSchema->GetNumAtts();
        int rightAttributesSize = right->nodeSchema->GetNumAtts();
        int currentAttributesSize = leftAttributesSize + rightAttributesSize;
        Attribute *currentAttributes = new Attribute[currentAttributesSize];

        Attribute *leftAttribute = left->nodeSchema->GetAtts();
        for (int countAttr = 0; countAttr < leftAttributesSize; countAttr++) {
            currentAttributes[countAttr].myType = leftAttribute[countAttr].myType;
            currentAttributes[countAttr].name = strdup(leftAttribute[countAttr].name);
        }
        Attribute *rightAttribute = right->nodeSchema->GetAtts();
        for (int countAttr = 0; countAttr < rightAttributesSize; countAttr++) {
            currentAttributes[leftAttributesSize + countAttr].myType = rightAttribute[countAttr].myType;
            currentAttributes[leftAttributesSize + countAttr].name = strdup(rightAttribute[countAttr].name);
        }
        Schema *currentNodeSchema = new Schema((char*) left->tableName.c_str(), currentAttributesSize, currentAttributes);

        delete []currentAttributes;
        if (PRINT_LEVEL == DEBUG) {
            cout << "Schema for join operation is as follows:" << endl;
            PrintSchema(currentNodeSchema);
            cout << endl;
        }

        if (0 == count) {

            forRelNameFormation.insert(make_pair(currentJoinNode->table1->tableName, count));
            forRelNameFormation.insert(make_pair(currentJoinNode->table2->tableName, count));

            //            currentJoin = getAndListForTable(boolean,currentJoinNode->table1,currentJoinNode->table2,false);
            //            cout<<" Print AndList for tables "<<currentJoinNode->table1->tableName<<" and "<<currentJoinNode->table2->tableName<<endl;
            //            PrintAndList(currentJoin);

            currentJoinList = getJoinListForJoin(left->nodeSchema, right->tableName);

            if (PRINT_LEVEL == DEBUG) {
                cout << "\n\n&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n\n";
                PrintAndList(currentJoinList);
                cout << "\n\n&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n\n";
                cout << endl;
            }
        } else {
            forRelNameFormationIterator = forRelNameFormation.find(currentJoinNode->table1->tableName);
            if (forRelNameFormationIterator == forRelNameFormation.end()) {
                currentJoinList = getJoinListForJoin(right->nodeSchema, currentJoinNode->table1->tableName);

                //table 1 not found insert table1
                forRelNameFormation.insert(make_pair(currentJoinNode->table1->tableName, count));
                //currentJoin = getAndListForTable(boolean,currentJoinNode->table1,currentJoinNode->table2,true);

                //                cout<<" Print AndList for primary table "<<currentJoinNode->table1->tableName<<" secondary table "<<currentJoinNode->table2->tableName<<endl;
            } else {
                //if table1 found than insert table2
                currentJoinList = getJoinListForJoin(left->nodeSchema, currentJoinNode->table2->tableName);

                forRelNameFormation.insert(make_pair(currentJoinNode->table2->tableName, count));
                //currentJoin = getAndListForTable(boolean, currentJoinNode->table2, currentJoinNode->table1, true);
                //                cout << " Print AndList for primary table " << currentJoinNode->table2->tableName << " secondary table " << currentJoinNode->table1->tableName << endl;
            }
            if (PRINT_LEVEL == DEBUG) {
                cout << "\n\n&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n\n";
                PrintAndList(currentJoinList);
                cout << "\n\n&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n\n";
                cout << endl;
            }
        }
        //generate CNF from AndList
        RelOpThreadData *data = new RelOpThreadData(6);
        CNF *cnf = new CNF;
        Record *literal = new Record;

        // constructs CNF predicate
        cnf->GrowFromParseTree(currentJoinList, left->nodeSchema, right->nodeSchema, *literal);

        if (PRINT_LEVEL == DEBUG) {
            cout << "CNF is as follows\n";
            cnf->Print();
        }

        if (left->operType == JOIN)
            data->setAtt(1, left->data->getAtt(3));
        else
            data->setAtt(1, left->data->getAtt(2));

        if (right->operType == JOIN)
            data->setAtt(2, right->data->getAtt(3));
        else
            data->setAtt(2, right->data->getAtt(2));

        Pipe *outPipe = new Pipe(PIPE_SIZE);
        data->setAtt(3, outPipe);

        data->setAtt(4, cnf);
        data->setAtt(5, literal);
        data->setAtt(6, &RELOP_PAGESIZE);

        string newTableName = left->tableName + +"." + right->tableName;
        TreeNode *currentNode = new TreeNode(JOIN, data, newTableName, currentNodeSchema, left->outPipe, right->outPipe, PipeIdCount++);

        currentNode->left = left;
        currentNode->right = right;

        //update the tableTreeNode hashMap to reflect new join
        forRelNameFormationIterator = forRelNameFormation.begin();
        //print all the attributes
        for (; forRelNameFormationIterator != forRelNameFormation.end(); ++forRelNameFormationIterator) {
            mapIt = tableTreeNode.find(forRelNameFormationIterator->first);
            if (mapIt != tableTreeNode.end()) {
                if (PRINT_LEVEL == DEBUG)
                    cout << "Entry erased " << mapIt->first << endl;
                tableTreeNode.erase(mapIt);
            }
            tableTreeNode.insert(make_pair(forRelNameFormationIterator->first, currentNode));
            if (PRINT_LEVEL == DEBUG)
                cout << "Entry created to point to new Join " << forRelNameFormationIterator->first << endl;
        }
    }
}

TableInfo* Parser::FetchTableInfo(string primaryTable) {
    for (int count = 0; count < tableInfoCounter; count++) {
        if (schemaForAttributesFormation[count].tableName.compare(primaryTable) == 0) {
            return &schemaForAttributesFormation[count];
        }
    }
    cout << "Table info for table " << primaryTable << " not found";
    exit(1);
}

AndList* Parser::getJoinListForJoin(Schema *joinSchema, string primaryTable) {

    //fetch the AndList

    TableInfo *primaryTableInfo = FetchTableInfo(primaryTable);
    //traverse the AndList
    AndList *tempAnd = primaryTableInfo->andListTable;
    Schema *primarySchema = primaryTableInfo->tableSchema;

    if (PRINT_LEVEL == DEBUG) {
        cout << "Primary schema is as follows" << endl;
        PrintSchema(primarySchema);
        cout << "Join schema is as follows" << endl;
        PrintSchema(joinSchema);
        cout << "Primary andList is as follows" << endl;
        PrintAndList(tempAnd);
    }
    bool joinTablesMatch;
    bool primaryTableMatch;
    AndList *returnAndList = NULL, *returnAndListEnd = NULL;
    while (tempAnd != NULL) {

        OrList *inputOr = tempAnd->left;
        OrList *tempOr = inputOr;
        primaryTableMatch = false;
        joinTablesMatch = false;
        while (tempOr != NULL) {
            struct ComparisonOp *inputCom = tempOr->left;

            if (inputCom->left->code == NAME) {
                if (primarySchema->Find(inputCom->left->value) != -1) {
                    primaryTableMatch = true;
                } else if (joinSchema->Find(inputCom->left->value) != -1) {
                    joinTablesMatch = true;
                } else {
                    //                    cout<<"Attriubte "<<inputCom->left->value<<" is not found in joinList and table, so skip the attribute"<<endl;
                    break; //skip the orList
                }
            }
            if (inputCom->right->code == NAME) {
                if (primarySchema->Find(inputCom->right->value) != -1) {
                    primaryTableMatch = true;
                } else if (joinSchema->Find(inputCom->right->value) != -1) {
                    joinTablesMatch = true;
                } else {
                    //                    cout<<"Attriubte "<<inputCom->right->value<<" is not found in joinList and table, so skip the attribute"<<endl;
                    break; //skip the orList
                }
            }
            tempOr = tempOr->rightOr;
        }
        if (joinTablesMatch) {
            //            cout<<"Match found "<<primaryTable<<endl;
            //            cout<<"\n===============================\n";
            //            Parser::PrintOrList(inputOr);
            //            cout<<"\n===============================\n";
            AndList *temp1 = new AndList;
            //cout<<"First Time"<<endl;
            temp1->rightAnd = NULL;
            temp1->left = inputOr;
            //copy the inputOr into temp

            if (returnAndList != NULL) {
                returnAndListEnd->rightAnd = temp1;
                returnAndListEnd = temp1;
            } else {
                returnAndList = temp1;
                returnAndListEnd = returnAndList;
            }
        }
        tempAnd = tempAnd->rightAnd;
    }
    return returnAndList;
}

void Parser::PrintOperand(struct Operand *pOperand) {
    if (pOperand != NULL) {
        cout << pOperand->value << " ";
    } else
        return;
}

void Parser::PrintComparisonOp(struct ComparisonOp *pCom) {
    if (pCom != NULL) {
        PrintOperand(pCom->left);
        switch (pCom->code) {
            case LESS_THAN:
                cout << " < ";
                break;
            case GREATER_THAN:
                cout << " > ";
                break;
            case EQUALS:
                cout << " = ";
        }
        PrintOperand(pCom->right);

    } else {
        return;
    }
}

void Parser::PrintOrList(struct OrList *pOr) {
    if (pOr != NULL) {
        struct ComparisonOp *pCom = pOr->left;
        PrintComparisonOp(pCom);

        if (pOr->rightOr) {
            cout << " OR ";
            PrintOrList(pOr->rightOr);
        }
    } else {
        return;
    }
}

void Parser::PrintAndList(AndList *pAnd) {
    if (pAnd != NULL) {
        cout << " (";
        struct OrList *pOr = pAnd->left;
        PrintOrList(pOr);
        cout << " )";
        if (pAnd->rightAnd) {
            cout << " AND ";
            PrintAndList(pAnd->rightAnd);
        }

    } else {
        return;
    }
}

void Parser::InsertNode(string tableName, TreeNode *node) {
    if (PRINT_LEVEL == DEBUG)
        cout << "Node for " << tableName << " inserted successfully\n";
    tableTreeNode.insert(make_pair(tableName, node));
}

AndList* Parser::getAndListForTable(AndList* inputAnd, TableInfo* table1, TableInfo* table2, bool join) {

    AndList *temp = NULL;
    AndList *andListEnd = NULL;
    bool result = false;
    while (inputAnd != NULL) {

        OrList *inputOr = inputAnd->left;
        result = TraversingOrList(inputOr, table1, table2, join);

        if (result) {
            AndList *temp1 = new AndList;
            //cout<<"First Time"<<endl;
            temp1->rightAnd = NULL;
            temp1->left = inputOr;
            //copy the inputOr into temp

            if (temp != NULL) {
                andListEnd->rightAnd = temp1;
                andListEnd = temp1;
            } else {
                temp = temp1;
                andListEnd = temp;
            }
        }
        inputAnd = inputAnd->rightAnd;
    }
    return temp;
}

bool Parser::TraversingOrList(OrList *inputOr, TableInfo *table1, TableInfo *table2, bool join) {

    int returnValue = 0;
    bool table1Match = false;
    while (inputOr != NULL) {
        struct ComparisonOp *inputCom = inputOr->left;
        returnValue = CheckComparisonOp(inputCom, table1, table2);
        if (0 == returnValue)
            return false;

        inputOr = inputOr->rightOr;
        if (join == false)
            continue;
        else if (1 == returnValue || returnValue == 3)
            table1Match = true;
    }
    if (join == false)
        return true;
    else if (join) {
        // return false if table1Match is true and return true if table1 match is true
        return table1Match;
    }
}

/*
 * This function return
 * 0 if left or right attribute is NAME and it is not found in table 1 and tabl2
 * 1 if left or right attribute is matched with table1 only
 * 2 if left or right attribute is matched with table2 only
 * 3 if one attribute is matched with table1 and other attribute is matched with table2 or vice versa
 */
int Parser::CheckComparisonOp(struct ComparisonOp *inputCom, TableInfo *table1, TableInfo *table2) {

    int returnValue = 0;
    if (inputCom != NULL) {

        if (inputCom->left->code == NAME) {
            if (table1->tableSchema->Find(inputCom->left->value) == -1) {

                if (table2 == NULL)
                    return 0;
                if (table2->tableSchema->Find(inputCom->left->value) == -1) {
                    return 0;
                } else
                    returnValue = 2;
            } else
                returnValue = 1;
        }

        if (inputCom->right->code == NAME) {
            if (table1->tableSchema->Find(inputCom->right->value) == -1) {

                if (table2 == NULL)
                    return false;

                if (table2->tableSchema->Find(inputCom->right->value) == -1) {
                    return false;
                } else {
                    if (returnValue == 1)
                        return 3;
                    else
                        return 2;
                }
            } else {
                if (returnValue == 2)
                    return 3;
                else
                    return 1;
            }
        }
        return returnValue;
    }
    return 0;
}

void Parser::insertAtt(Schema *schema, char *attribute) {
    //search for this attribute in each table
    int result;

    //PrintSchema(schema);
    result = schema->Find(attribute);
    //cout << result << endl;
    if (result != -1) {
        if (PRINT_LEVEL == DEBUG) {
            cout << " Attribute inserted" << attribute << endl;
        }
        insertAtt(result);
        return;
    }
    //    Attribute *myAtts = schema->GetAtts();
    //    int numAtts = schema->GetNumAtts();
    //    for (int i = 0; i < numAtts; i++) {
    //        if (!strcmp(attribute, myAtts[i].name)) {
    //            insertAtt(i);
    //            break;
    //        }
    //        cout <<i<<" attName" << " " << myAtts[i].name<<endl;
    //    }
    //check if its appended with alias or table name
    cout << "Attribute " << attribute << " not found in the table list" << endl;
    cout << "Exiting the system\n";
    exit(1);
}

TableInfo* Parser::FindAttInTable(char *attribute, TableInfo *schemaForAttributesFormation, int counter) {
    //search for this attribute in each table
    int result;
    for (int count = 0; count < counter; count++) {
        result = schemaForAttributesFormation[count].tableSchema->Find(attribute);

        if (result != -1)
            return &schemaForAttributesFormation[count];
    }
    cout << "Attribute " << attribute << " not found in the table list" << endl;
    cout << "Exiting the system\n";
    exit(1);
}

void Parser::insertAtt(int attNum) {

    /*
     * search for attribute in the array first attToKeep for project operation
     * Since array is sorted so search till you reach value equal or greater
     */
	printMsg("Parser::insertAtt - number of attributes: ", numberOfAttributes );
    int count;
    for (count = 0; count < numberOfAttributes; count++) {

        //if found return parameter already exist
        if (attNum == attsToKeep[count]) {
            return;
        }
        if (attNum < attsToKeep[count]) {
            break;
        }
    }
    for (int count1 = numberOfAttributes; count1 > count; count1--) {
        attsToKeep[count1] = attsToKeep[count1 - 1];
    }
    attsToKeep[count] = attNum;
    numberOfAttributes++;

        printMsg("Parser::insertAtt - AttsToKeep is as follows", "");

        for (count = 0; count < numberOfAttributes; count++) {
		printMsg("", attsToKeep[count] );
        }
}

void Parser::generateSortArray(int *attsToKeep, int numberOfAttributes, int attNum) {

    /*
     * search for attribute in the array first attToKeep for project operation
     * Since array is sorted so search till you reach value equal or greater
     */

        printMsg("Parser::generateSortArray- number of attribute: ", numberOfAttributes);
    int count;
    for (count = 0; count < numberOfAttributes; count++) {

        //if found return parameter already exist
        if (attNum == attsToKeep[count]) {
            return;
        }
        if (attNum < attsToKeep[count]) {
            break;
        }
    }
    for (int count1 = numberOfAttributes; count1 > count; count1--) {
        attsToKeep[count1] = attsToKeep[count1 - 1];
    }
    attsToKeep[count] = attNum;
    numberOfAttributes++;

    if (PRINT_LEVEL == DEBUG) {
        cout << "AttsToKeep is as follows\n";

        for (count = 0; count < numberOfAttributes; count++) {
            cout << attsToKeep[count] << " ";
        }
        cout << endl;
    }
}

void Parser::generateAtts(Schema *tableSchema) {

    Attribute *origAtts = tableSchema->GetAtts();

    projectAtts = new Attribute[numberOfAttributes];

    for (int count = 0; count < numberOfAttributes; count++) {
        projectAtts[count].name = strdup(origAtts[attsToKeep[count]].name);
        projectAtts[count].myType = origAtts[attsToKeep[count]].myType;
    }
    projectSchema = new Schema((char*) "Project", numberOfAttributes, projectAtts);
}

void Parser::findLeftDeepJoinOrder() {

	printMsg("Parser::findLeftDeepJoinOrder - Optimizing join......", "");
    minEstimate = -1;
    EnumerateJoin(boolean, schemaForAttributesFormation, tableInfoCounter);
        printMsg("Parser::findLeftDeepJoinOrder - Enumerate Joins generated. Join size is ", joinList.size() ); 
    //Now generate permutation to get the best left deep join order
    generatePermutation(joinList.size());
}

void Parser::EnumerateJoin(AndList* inputAnd, TableInfo *schemaForAttributesFormation, int counter) {

    AndList * tempAndList = inputAnd;
    while (tempAndList != NULL) {

        OrList *inputOr = tempAndList->left;

        if (inputOr->rightOr == NULL) {
            ComparisonOp *temp = inputOr->left;

            if (temp->left->code == NAME && temp->right->code == NAME && temp->code == EQUALS) {
                //search for left and right attribute and get the table names for these attributes
                TableInfo * leftTableIn = FindAttInTable(temp->left->value, schemaForAttributesFormation, counter);
                TableInfo * rightTableIn = FindAttInTable(temp->right->value, schemaForAttributesFormation, counter);
                AndList *tempAndList = new AndList;

                tempAndList->left = inputOr;
                tempAndList->rightAnd = NULL;


                //once you got the attribute
                // call the function to get AndList correspond to table1 and table2
                AndList* andListJoin = getAndListForTable(inputAnd, leftTableIn, rightTableIn, false);

                JoinNode *tempJoin = new JoinNode(tempAndList, andListJoin, leftTableIn, rightTableIn);
                joinList.push_back(tempJoin);
                if (PRINT_LEVEL == DEBUG) {
                    cout << "Join found\n";
                    PrintAndList(tempJoin->joinAndList);
                    cout << endl;

                    cout << "Table1: " << leftTableIn->tableName << endl;
                    cout << "Table2: " << rightTableIn->tableName << endl;
                    cout << "AndList for Estimate or Apply for this Join operation is as follows\n";
                    PrintAndList(tempJoin->estimateAndList);
                }
            }
        }
        tempAndList = tempAndList->rightAnd;
    }
}

void Parser::generatePermutation(int size) {
	printMsg("Parser::generatePermutation - Generating optimized permutation...................", "");
    //generate the number of permutation
    char *permuteString = new char[size + 1];
    for (int count = 0; count < size; count++) {
        permuteString[count] = 97 + count;
    }
    permuteString[size] = '\0';

    char *out = new char[size + 1];
    out[size] = '\0';
    int *used = new int[size];

    for (int i = 0; i < size; i++) {
        used[i] = 0;
    }
    DoPermute(permuteString, out, used, size, 0);
}

void Parser::DoPermute(char in[], char out[], int used[], int length, int recursLev) {

	printMsg("Parser::DoPermute- Permuting.......................................", "");
    int i;

    /*Base Case*/
    if (recursLev == length) {
        //cout << out << endl;
        if (checkInOrder(out)) {
            if (PRINT_LEVEL == DEBUG)
                cout << " Valid Deep Join Order " << out << endl;
            joinOrder(out);
        } else {
            if (PRINT_LEVEL == DEBUG)
                cout << " Invalid Deep Join Order\n";
        }
    }

    /*Recursive Case*/
    for (i = 0; i < length; i++) {
        if (used[i]) {
            continue;
        }

        out[recursLev] = in[i];
        used[i] = 1;
        DoPermute(in, out, used, length, recursLev + 1);
        used[i] = 0;

    }
}

bool Parser::checkInOrder(char *permuteOrder) {

    std::tr1::unordered_map<string, TableInfo*> joinTableInfoArray;
    std::tr1::unordered_map<string, TableInfo*>::iterator joinTableInfoArrayIterator;
    for (int count = 0; count < joinList.size(); count++) {
        int currentOrder = permuteOrder[count] - 97;

        if (PRINT_LEVEL == DEBUG)
            cout << "Current order is " << currentOrder << endl;
        if (count == 0) {
            //insert into HashMap
            joinTableInfoArray.insert(make_pair(joinList[currentOrder]->table1->tableName, joinList[currentOrder]->table1));
            joinTableInfoArray.insert(make_pair(joinList[currentOrder]->table2->tableName, joinList[currentOrder]->table2));
        } else {
            //check for match in HashMap
            joinTableInfoArrayIterator = joinTableInfoArray.find(joinList[currentOrder]->table1->tableName);

            if (joinTableInfoArrayIterator != joinTableInfoArray.end()) {
                joinTableInfoArray.insert(make_pair(joinList[currentOrder]->table2->tableName, joinList[currentOrder]->table2));
            } else {
                joinTableInfoArrayIterator = joinTableInfoArray.find(joinList[currentOrder]->table2->tableName);
                if (joinTableInfoArrayIterator != joinTableInfoArray.end()) {
                    joinTableInfoArray.insert(make_pair(joinList[currentOrder]->table1->tableName, joinList[currentOrder]->table1));
                } else {
                    return false;
                }
            }
        }
    }
    return true;
}

long Parser::joinOrder(char *permuteOrder) {

    std::tr1::unordered_map<string, int> forRelNameFormation;
    std::tr1::unordered_map<string, int>::iterator forRelNameFormationIterator;
    char **relName;
    double joinEstimate;
    Statistics statObj;
    InitStatObject(statObj);
    relName = new char*[joinList.size() + 1];

    for (int count = 0; count < joinList.size(); count++) {
        int currentOrder = permuteOrder[count] - 97;

        //fetch the and list for joinList[currentOrder] for table1 and table2
        JoinNode *currentJoinNode = joinList[currentOrder];

        AndList *currentJoin;
        if (0 == count) {

            forRelNameFormation.insert(make_pair(currentJoinNode->table1->tableName, count));
            forRelNameFormation.insert(make_pair(currentJoinNode->table2->tableName, count));

            relName[0] = new char [strlen(currentJoinNode->table1->tableName.c_str())];
            strcpy(relName[0], currentJoinNode->table1->tableName.c_str());
            relName[1] = new char [strlen(currentJoinNode->table2->tableName.c_str())];
            strcpy(relName[1], currentJoinNode->table2->tableName.c_str());

            //currentJoin = getAndListForTable(boolean,tempNode->table1,tempNode->table2,false);
            //if (PRINT_LEVEL == DEBUG) {
	        printMsg("Parser:joinOrder - And List for joined table: ", "");
                callFunction("", PrintAndList(currentJoinNode->estimateAndList) );

		printMsg("Parser::joinOrder - Table Name: ", "");
                for (int i = 0; i < count + 2; i++) {
		    printMsg("", relName[i]);
                }
            //}

            if (count == joinList.size() - 1) {
                joinEstimate = statObj.Estimate(currentJoinNode->estimateAndList, relName, count + 2);
                    printMsg("Parser::joinOrder - Join estimate is: ",  joinEstimate);;
            } else {
                    joinEstimate = statObj.Estimate(currentJoinNode->estimateAndList, relName, count + 2);
                    printMsg("Parser::joinOrder - Join estimate before apply is: ",  joinEstimate);;
                statObj.Apply(currentJoinNode->estimateAndList, relName, count + 2);
            }
        } else {
            forRelNameFormationIterator = forRelNameFormation.find(currentJoinNode->table1->tableName);

            if (forRelNameFormationIterator == forRelNameFormation.end()) {

                //if table 1 not found then insert table1
                relName[count + 1] = new char [strlen(currentJoinNode->table1->tableName.c_str())];
                strcpy(relName[count + 1], currentJoinNode->table1->tableName.c_str());
                currentJoin = getAndListForTable(boolean, currentJoinNode->table1, currentJoinNode->table2, true);

                //                cout<<" Print AndList for primary table "<<currentJoinNode->table1->tableName<<" secondary table "<<currentJoinNode->table2->tableName<<endl;
            } else {

                //if table1 found than insert table2
                relName[count + 1] = new char [strlen(currentJoinNode->table2->tableName.c_str())];
                strcpy(relName[count + 1], currentJoinNode->table2->tableName.c_str());

                currentJoin = getAndListForTable(boolean, currentJoinNode->table2, currentJoinNode->table1, true);
                //                cout << " Print AndList for primary table " << currentJoinNode->table2->tableName << " secondary table " << currentJoinNode->table1->tableName << endl;

            }
            if (PRINT_LEVEL == DEBUG) {
                PrintAndList(currentJoin);
                cout << endl;
                cout << " Table name in relNames\n";
                for (int i = 0; i < joinList.size() + 1; i++) {
                    cout << relName[i] << " " << endl;
                }
            }


            if (count == joinList.size() - 1) {
                joinEstimate = statObj.Estimate(currentJoin, relName, count + 2);
                if (PRINT_LEVEL == DEBUG)
                    cout << "$$$$$$$$$$$$$$$$$$$Join estimate is " << joinEstimate << endl;
            } else {
                if (PRINT_LEVEL == DEBUG) {
                    joinEstimate = statObj.Estimate(currentJoin, relName, count + 2);
                    cout << "$$$$$$$$$$$$$$$$$$Join estimate before apply is " << joinEstimate << endl;
                }
                statObj.Apply(currentJoin, relName, count + 2);
            }
        }
    }
    if (PRINT_LEVEL == DEBUG)
        cout << "Minimum is " << minEstimate << "\nEstimate is " << joinEstimate << endl;

    if (joinEstimate < minEstimate || minEstimate == -1) {

        minEstimate = joinEstimate;
        minJoinCombination = permuteOrder;
        if (PRINT_LEVEL == DEBUG) {
            cout << "Modifying the minimum value\n";
            cout << "Permute Order " << permuteOrder << endl;
            cout << "MinJoinCombination " << minJoinCombination << endl;
        }
    }
}

void Parser::CheckJoinValidity() {
    TreeNodeMap::iterator mapIt = tableTreeNode.begin();

    TreeNode *node = NULL;

    if (PRINT_LEVEL == DEBUG)
        cout << "Current state of tree is as follows:" << endl;

    //checking all the table entries to verify whether only one node left or not
    for (; mapIt != tableTreeNode.end(); ++mapIt) {
        TreeNode * temp = mapIt->second;

        if (PRINT_LEVEL == DEBUG)
            cout << "Oper Type:3 is for join" << temp->operType << " TableName " << temp->tableName << endl;
        if (node == NULL)
            node = temp;
        else if (temp != node) {
            cout << "There are still nodes left in the system after the join operation. Exiting the system" << endl;
            exit(1);
        }
    }
}

//This will execute the query plan 
void Parser::Execute() {
    printMsg("Parser::Execute - Executing query plan....................", "");
    TreeNodeMap::iterator mapIt = tableTreeNode.begin();
    TreeNode *node = mapIt->second;

    printMsg("Parser::Execute - Root node: ", node->operType );

    //print the node before execution
    cout << "\n\n\n Below is logical query plan: \n";
    node->PrintNode();
    cout<<"\n\n";
    node->Execute();
}

void Parser::PrintSchema(Schema *incomingSchema) {
    Attribute *temp = incomingSchema->GetAtts();
    for (int count = 0; count < incomingSchema->GetNumAtts(); count++) {
        cout << "Name " << temp[count].name << " Type " << temp[count].myType << endl;
    }
}

void Parser::PrintTreeNode() {
    cout << "--------------------------\n";
    //print the tree map

    TreeNodeMap::iterator mapIt = tableTreeNode.begin();
    //print all the tables
    for (; mapIt != tableTreeNode.end(); ++mapIt) {
        cout << "The Table Entry " << mapIt->first << endl;
        TreeNode *temp = mapIt->second;
        cout << "NumAtts " << temp->nodeSchema->GetNumAtts() << endl;
        PrintSchema(temp->nodeSchema);
    }
    cout << "---------------------------\n";
}



/* This removes the table schema from catalog */
void Parser::deleteTable(TableProperties *tablesProp) {
    FILE *foo = fopen(catalog_path, "r");
    FILE *fooTemp = fopen("tempCatalog", "w");


    // this is enough space to hold any tokens
    char space[200];
    char *relName;
    int sizeOfTableName = strlen(tablesProp->tableName);
    relName = tablesProp->tableName;

    bool deleteLine = false, deleteDone = false;

    while (fgets(space, 200, foo) != NULL) {

        if (deleteDone) {
            fprintf(fooTemp, "%s", space);
        }
        if (strncmp(space, "BEGIN", strlen("BEGIN") - 1)) {
            if (deleteLine == false)
                fprintf(fooTemp, "%s", space);

        } else {
            fgets(space, 200, foo);
            if (strncmp(space, relName, strlen(relName) - 1) == 0) {
                deleteLine = true;
            } else {
                fprintf(fooTemp, "BEGIN\n");
                fprintf(fooTemp, "%s", space);
                if (deleteLine) {
                    deleteDone = true;
                    deleteLine = false;
                }
            }
        }
    }
    if(deleteDone)
        cout<<"Table "<<tablesProp->tableName<<" dropped succcessfully\n";
    else{
	cerr<<"Table "<<tablesProp->tableName<<" not found in catalog ! Exiting....."<<endl;
	exit(0);
    }
    fclose(fooTemp);
    fclose(foo);
    rename("tempCatalog", catalog_path);
    remove("tempCatalog");
}

/* This will insert a new table into catalog */
void Parser::insertIntoTable(TableProperties *tablesProp) {
   printMsg("Parser::insertInToTable - Inserting table into catalog: ", tablesProp->tableName);

    char tbl_path[100];
    sprintf(tbl_path, "%s%s.tbl", tpch_dir, tablesProp->fileName->value); //create path

    DBFile dbfile;
    char bin_path[100];
    sprintf(bin_path, "%s%s.bin", dbfile_dir, tablesProp->tableName);
    dbfile.Open(bin_path);

    Schema *tempSchema = new Schema(catalog_path, tablesProp->tableName);
   printMsg("Parser::insertInToTable - TPCH path: ", tbl_path ); 
    dbfile.Load(*tempSchema, tbl_path);
    dbfile.Close();
    cout<<"Table "<<tablesProp->tableName<<" inserted successfully !"<<endl;
}

void Parser::InitStatObject(Statistics &statObj) {
    char *fileName = "Statistics.txt";
    char *relName[] = {(char*) "nation", (char*) "region", (char*) "customer", (char*) "part", (char*) "supplier", (char*) "partsupp", (char*) "orders", (char*) "lineitem"};

    statObj.AddRel(relName[0], 25);
    statObj.AddAtt(relName[0], (char*) "n_nationkey", 25);
    statObj.AddAtt(relName[0], (char*) "n_regionkey", 5);
    statObj.AddAtt(relName[0], (char*) "n_name", 25);
    statObj.AddAtt(relName[0], (char*) "n_comment", 25);

    statObj.AddRel(relName[1], 5);
    statObj.AddAtt(relName[1], (char*) "r_regionkey", 5);
    statObj.AddAtt(relName[1], (char*) "r_name", 5);
    statObj.AddAtt(relName[1], (char*) "r_comment", 5);

    statObj.AddRel(relName[2], 150000);
    statObj.AddAtt(relName[2], (char*) "c_custkey", 150000);
    statObj.AddAtt(relName[2], (char*) "c_name", 150000);
    statObj.AddAtt(relName[2], (char*) "c_address", 150000);
    statObj.AddAtt(relName[2], (char*) "c_phone", 150000);
    statObj.AddAtt(relName[2], (char*) "c_acctbal", 140187);
    statObj.AddAtt(relName[2], (char*) "c_nationkey", 25);
    statObj.AddAtt(relName[2], (char*) "c_mktsegment", 5);
    statObj.AddAtt(relName[2], (char*) "c_comment", 149965);

    statObj.AddRel(relName[3], 200000);
    statObj.AddAtt(relName[3], (char*) "p_partkey", 200000);
    statObj.AddAtt(relName[3], (char*) "p_name", 199996);
    statObj.AddAtt(relName[3], (char*) "p_size", 50);
    statObj.AddAtt(relName[3], (char*) "p_mfgr", 5);
    statObj.AddAtt(relName[3], (char*) "p_comment", 127459);
    statObj.AddAtt(relName[3], (char*) "p_retailprice", 20899);
    statObj.AddAtt(relName[3], (char*) "p_container", 40);
    statObj.AddAtt(relName[3], (char*) "p_type", 150);
    statObj.AddAtt(relName[3], (char*) "p_brand", 25);

    statObj.AddRel(relName[4], 10000);
    statObj.AddAtt(relName[4], (char*) "s_suppkey", 10000);
    statObj.AddAtt(relName[4], (char*) "s_name", 10000);
    statObj.AddAtt(relName[4], (char*) "s_address", 10000);
    statObj.AddAtt(relName[4], (char*) "s_nationkey", 25);
    statObj.AddAtt(relName[4], (char*) "s_phone", 10000);
    statObj.AddAtt(relName[4], (char*) "s_acctbal", 9955);
    statObj.AddAtt(relName[4], (char*) "s_comment", 10000);

    statObj.AddRel(relName[5], 800000);
    statObj.AddAtt(relName[5], (char*) "ps_suppkey", 10000);
    statObj.AddAtt(relName[5], (char*) "ps_partkey", 200000);
    statObj.AddAtt(relName[5], (char*) "ps_availqty", 9999);
    statObj.AddAtt(relName[5], (char*) "ps_supplycost", 99865);
    statObj.AddAtt(relName[5], (char*) "ps_comment", 799123);

    statObj.AddRel(relName[6], 1500000);
    statObj.AddAtt(relName[6], (char*) "o_custkey", 99996);
    statObj.AddAtt(relName[6], (char*) "o_orderdate", 2406);
    statObj.AddAtt(relName[6], (char*) "o_orderkey", 1500000);
    statObj.AddAtt(relName[6], (char*) "o_totalprice", 1564556);
    statObj.AddAtt(relName[6], (char*) "o_orderstatus", 3);
    statObj.AddAtt(relName[6], (char*) "o_comment", 1478684);
    statObj.AddAtt(relName[6], (char*) "o_orderpriority", 5);
    statObj.AddAtt(relName[6], (char*) "o_clerk", 1000);
    statObj.AddAtt(relName[6], (char*) "o_shippriority", 1);

    statObj.AddRel(relName[7], 6001215);
    statObj.AddAtt(relName[7], (char*) "l_orderkey", 1500000);
    statObj.AddAtt(relName[7], (char*) "l_partkey", 200000);
    statObj.AddAtt(relName[7], (char*) "l_suppkey", 10000);
    statObj.AddAtt(relName[7], (char*) "l_linenumber", 7);
    statObj.AddAtt(relName[7], (char*) "l_quantity", 50);
    statObj.AddAtt(relName[7], (char*) "l_extendedprice", 933900);
    statObj.AddAtt(relName[7], (char*) "l_discount", 11);
    statObj.AddAtt(relName[7], (char*) "l_tax", 9);
    statObj.AddAtt(relName[7], (char*) "l_linestatus", 2);
    statObj.AddAtt(relName[7], (char*) "l_shipdate", 2526);
    statObj.AddAtt(relName[7], (char*) "l_commitdate", 2466);
    statObj.AddAtt(relName[7], (char*) "l_receiptdate", 2554);
    statObj.AddAtt(relName[7], (char*) "l_shipinstruct", 4);
    statObj.AddAtt(relName[7], (char*) "l_shipmode", 7);
    statObj.AddAtt(relName[7], (char*) "l_returnflag", 3);
    statObj.AddAtt(relName[7], (char*) "l_comment", 4501941);

    statObj.Write(fileName);
}
