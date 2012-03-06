/* File - debugFile.h
 * Debug Macro - to help in debugging
 */
#ifndef _DEBUGFILE_H
#define _DEBUGFILE_H

#include <iostream>

//#define DEBUG_F

#ifdef DEBUG_F
	#define printMsg(msg, value)\
	cerr<<__FILE__<<":"<<msg<<value<<endl;

	#define callFunction(msg, function)\
	cerr<<__FILE__<<":"<<msg<<endl;\
	function;
#else
	#define printMsg(msg, value)
	#define callFunction(msg, function)
#endif

#endif
