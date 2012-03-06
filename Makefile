CC = g++ -O2 -Wno-deprecated 

tag = -i

ifdef linux
tag = -n
endif

main: CommonClass.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o GenericDBFile.o Parser.o HeapFile.o SortedFile.o DBFile.o Pipe.o BigQ.o RelOp.o Function.o Statistics.o y.tab.o lex.yy.o main.o
	$(CC) -o main CommonClass.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o GenericDBFile.o Parser.o HeapFile.o SortedFile.o DBFile.o Pipe.o BigQ.o RelOp.o Function.o Statistics.o y.tab.o lex.yy.o main.o -lfl -lpthread

main.o : main.cc
	$(CC) -g -c main.cc

CommonClass.o: CommonClass.cc
	$(CC) -g -c CommonClass.cc

Pipe.o: Pipe.cc
	$(CC) -g -c Pipe.cc

BigQ.o: BigQ.cc
	$(CC) -g -c BigQ.cc

RelOp.o: RelOp.cc
	$(CC) -g -c RelOp.cc

Function.o: Function.cc
	$(CC) -g -c Function.cc

Parser.o: Parser.cc
	$(CC) -g -c Parser.cc
	
GenericDBFile.o: GenericDBFile.cc
	$(CC) -g -c GenericDBFile.cc

HeapFile.o: HeapFile.cc
	$(CC) -g -c HeapFile.cc

SortedFile.o: SortedFile.cc
	$(CC) -g -c SortedFile.cc

Statistics.o: Statistics.cc
	$(CC) -g -c Statistics.cc

Comparison.o: Comparison.cc
	$(CC) -g -c Comparison.cc
	
ComparisonEngine.o: ComparisonEngine.cc
	$(CC) -g -c ComparisonEngine.cc
	
DBFile.o: DBFile.cc
	$(CC) -g -c DBFile.cc

File.o: File.cc
	$(CC) -g -c File.cc

Record.o: Record.cc
	$(CC) -g -c Record.cc

Schema.o: Schema.cc
	$(CC) -g -c Schema.cc
	
y.tab.o: Parser.y
	yacc -d Parser.y
	sed $(tag) y.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" 
	g++ -c y.tab.c

lex.yy.o: Lexer.l
	flex  Lexer.l
	gcc  -c lex.yy.c

clean: 
	rm -f *.o
	rm -f *.out
	rm -f y.tab.c
	rm -f lex.yy.c
	rm -f y.tab.h
