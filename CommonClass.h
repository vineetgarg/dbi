#include "Defs.h"
#include "Comparison.h"
#include "debugFile.h"
#include <iostream>

#ifndef _COMMONCLASS_H
#define	_COMMONCLASS_H

struct header {
    OrderMaker *o;
    int l;
};

class HeaderInformation
{
    fType fileType;
    OrderMaker order;
    int runLength;
public:
    void setFileType(fType type);
    void setSortOrder(OrderMaker order);
    void setRunLength(int runLength);
    fType getFileType();
    OrderMaker* getSortOrder();
    int getRunLength();
};

/* this class encapsulate the data required to execute a relation operator
 * this will be used by parser to execute queries */
class RelOpThreadData {
    void **att;
    int numAtts;

public:
    RelOpThreadData(int numAtts);
    int setAtt(int attNum, void* inputAtt);
    void* getAtt(int attNum);

};
#endif	/* _COMMONCLASS_H */

