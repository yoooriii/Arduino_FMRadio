#include "RDSParser2.h"

RDSParser2::RDSParser2() {
  memset(this, 0, sizeof(RDSParser2));
} // RDSParser2()


void RDSParser2::init() {
	strcpy(_PSName1, "--------");
	strcpy(_PSName2, _PSName1);
	strcpy(programServiceName, "          ");
	memset(_RDSText, 0, sizeof(_RDSText));
	strcpy(_RDSText, _PSName1);
	_lastTextIDX = 0;
}

void RDSParser2::attachServicenNameCallback(receiveServicenNameFunction newFunction) {
}

void RDSParser2::attachTimeCallback(receiveTimeFunction newFunction) {
}

