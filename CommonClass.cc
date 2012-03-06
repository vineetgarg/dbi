#include "CommonClass.h"

void HeaderInformation::setFileType(fType type)
{
    this->fileType = type;
}

fType HeaderInformation::getFileType()
{
    return fileType;
}

void HeaderInformation::setSortOrder(OrderMaker order)
{
    this->order = order;
}

OrderMaker* HeaderInformation::getSortOrder()
{
    return &order;
}

void HeaderInformation::setRunLength(int runLength)
{
    this->runLength = runLength;
}

int HeaderInformation::getRunLength()
{
    return runLength;
}

RelOpThreadData::RelOpThreadData(int numAtts) {
    if (numAtts > 0) {
        this->numAtts = numAtts;
        att = new void*[numAtts];
    } else {
        numAtts = 0;
        att = NULL;
    }
}

int RelOpThreadData::setAtt(int attNum, void* inputAtt) {
    if (attNum <= numAtts && attNum > 0 && att !=NULL) {
        att[attNum - 1] = inputAtt;

        return 1;
    } else {
	    printMsg("RelOpThreadData::setAtt - attNum is invalid ! ", attNum);
        cerr << "Please pass the valid attribute number while setting the record. Attribute number passed is:"<<attNum<<" "<<numAtts<<endl;
        return 0;
    }
}

void* RelOpThreadData::getAtt(int attNum) {
    if (attNum <= numAtts && attNum > 0 && att !=NULL) {
        return att[attNum - 1];
    } else {
        cout << "Please pass the valid attribute number while setting the record. Attribute number passed is:"<<attNum<<" "<<numAtts<<endl;
        return NULL;
    }
}
