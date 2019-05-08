#include"SecondLevelTable.h"


SecondLevelTable::SecondLevelTable() {
	head = 0;
	fragment = 0;
}

PMTEntry& SecondLevelTable::operator[](unsigned index) {
	return pmtEntries[index];
}