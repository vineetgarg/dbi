/* File - main.cc
 * This file asks user for a query and pass it to the parser
 * @Vineet Garg
 * 21 April, 2011
 */
#include <iostream>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <typeinfo>
#include "Function.h"
#include "ParseTree.h"
#include "Parser.h"
#include "RelOp.h"
#include "debugFile.h"

using namespace std;

const char *settings = "test.cat"; //configuration file containing all the configuration
char *catalog_path, *dbfile_dir, *tpch_dir = NULL;
outputMode PRINT_LEVEL = OUTPUT;

extern "C" {
    int yyparse(void); // defined in y.tab.c
}

void traverseFunction(FuncOperator *tempFunctionAtts,TableInfo *schemaForAttributesFormation,int tableCounter);

//This function will setup the initial configuration
//this is the same code used by Professor in varous test drivers 
void setup() {
    FILE *fp = fopen(settings, "r");
    if (fp) {
        char *mem = (char *) malloc(80 * 3);
        catalog_path = &mem[0];
        dbfile_dir = &mem[80];
        tpch_dir = &mem[160];
        char line[80];
        fgets(line, 80, fp);
        sscanf(line, "%s\n", catalog_path);
        fgets(line, 80, fp);
        sscanf(line, "%s\n", dbfile_dir);
        fgets(line, 80, fp);
        sscanf(line, "%s\n", tpch_dir);
        fclose(fp);
        if (!(catalog_path && dbfile_dir && tpch_dir)) {
            cerr << " Test settings file 'test.cat' not in correct format.\n";
            free(mem);
            exit(1);
        }
    } else {
        cerr << " Test settings files 'test.cat' missing \n";
        exit(1);
    }
    cout << " \n** IMPORTANT: MAKE SURE THE INFORMATION BELOW IS CORRECT **\n";
    cout << " catalog location: \t" << catalog_path << endl;
    cout << " tpch files dir: \t" << tpch_dir << endl;
    cout << " heap files dir: \t" << dbfile_dir << endl;
    cout << " \n\n";
}

int main() {
    //for setting the folder options
    setup();

    extern FuncOperator *finalFunction; // the aggregate function (NULL if no agg)
    extern TableList *tables; // the list of tables and aliases in the query
    extern AndList *boolean; // the predicate in the WHERE clause
    extern NameList *groupingAtts; // grouping atts (NULL if no grouping)
    extern NameList *attsToSelect; // the set of attributes in the SELECT (NULL if no such atts)
    extern int distinctAtts; // 1 if there is a DISTINCT in a non-aggregate query
    extern int distinctFunc; // 1 if there is a DISTINCT in an aggregate query
    extern struct TableProperties *tablesProp;
    extern int fileType;
   
    cout<<"Enter a query: ";
    yyparse();

    callFunction("Query's andlist: ", Parser::PrintAndList(boolean) );
    if (tablesProp!=NULL) {

        Parser *parseQuery = new Parser;
        if (tablesProp->start!=NULL){
            //parseQuery->createTable(tablesProp,fileType,attsToSelect); //create query
        }
        else{
            if(tablesProp->fileName==NULL){
                parseQuery->deleteTable(tablesProp); //delete table query
            }
            else {
                parseQuery->insertIntoTable(tablesProp);; //drop table query
            }
        }

    } else {
        Parser parseQuery(finalFunction, tables, boolean, groupingAtts, attsToSelect, distinctAtts, distinctFunc); //initialize the query parser 

        parseQuery.QueryParsing(); //parse the query
    }
}
