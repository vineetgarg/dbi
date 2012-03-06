#ifndef DEFS_H
#define DEFS_H


#define MAX_ANDS 20
#define MAX_ORS 20
#define INDEXOFFSET 1
#define PAGE_SIZE 131072
const int PIPE_SIZE =100; // pipe cache size

enum Target {Left, Right, Literal};
enum CompOperator {LessThan, GreaterThan, Equals};
enum Type {Int, Double, String};
typedef enum {heap, sorted, tree} fType;

typedef enum {READ, WRITE} fileMode;

unsigned int Random_Generate();

#define HEADERFILE_LENGTH 7
#define HEADERFILE_EXT ".header"
#define INDEXOFFSET 1
#define RAND_SIZE 6

typedef enum {DEBUG, ERROR, OUTPUT} outputMode;
extern outputMode PRINT_LEVEL;
typedef enum {SELECT_FILE, SELECT_PIPE,PROJECT,JOIN,DUPLICATE_REMOVAL,SUM,GROUPBY,WRITEOUT} relOpMode;

#endif

