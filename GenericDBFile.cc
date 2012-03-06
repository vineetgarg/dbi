#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "GenericDBFile.h"
#include "Defs.h"

GenericDBFile::GenericDBFile() {
}
GenericDBFile::~GenericDBFile(){
}

int GenericDBFile::Create (char *f_path, fType f_type, void *startup) {
}

void GenericDBFile::Load (Schema &f_schema, char *loadpath) {
}

int GenericDBFile::Open (char *f_path) {
}

void GenericDBFile::MoveFirst () {
}

int GenericDBFile::Close () {
}

void GenericDBFile::Add (Record &rec) {
}

int GenericDBFile::GetNext (Record &fetchme) {
	return 0;
}

int GenericDBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
}
