
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     Name = 258,
     Float = 259,
     Int = 260,
     String = 261,
     CREATE = 262,
     ON = 263,
     HEAP = 264,
     SORTED = 265,
     DROP = 266,
     INSERT = 267,
     INTO = 268,
     TABLE = 269,
     INTEGER = 270,
     DOUB = 271,
     STR = 272,
     SELECT = 273,
     GROUP = 274,
     DISTINCT = 275,
     BY = 276,
     FROM = 277,
     WHERE = 278,
     SUM = 279,
     AS = 280,
     AND = 281,
     OR = 282
   };
#endif
/* Tokens.  */
#define Name 258
#define Float 259
#define Int 260
#define String 261
#define CREATE 262
#define ON 263
#define HEAP 264
#define SORTED 265
#define DROP 266
#define INSERT 267
#define INTO 268
#define TABLE 269
#define INTEGER 270
#define DOUB 271
#define STR 272
#define SELECT 273
#define GROUP 274
#define DISTINCT 275
#define BY 276
#define FROM 277
#define WHERE 278
#define SUM 279
#define AS 280
#define AND 281
#define OR 282




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 27 "Parser.y"

 	struct FuncOperand *myOperand;
	struct FuncOperator *myOperator; 
	struct TableList *myTables;
        char *mychar;
	struct ComparisonOp *myComparison;
	struct Operand *myBoolOperand;
	struct OrList *myOrList;
	struct AndList *myAndList;
	struct NameList *myNames;
        struct AttributeList *myAttributeList;
        struct Attr *myAttribute;
        struct TableProperties *myTable;
	char *actualChars;
	char whichOne;



/* Line 1676 of yacc.c  */
#line 125 "y.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


