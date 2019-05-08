#ifndef _SECOND_LEVEL_TABLE_H_
#define _SECOND_LEVEL_TABLE_H_

#include "vm_declarations.h"
#include "PmtEntry.h"

class PMTEntry;
class KernelSystem;

class SecondLevelTable {
public:
	SecondLevelTable();

	PMTEntry& operator[](unsigned index);

private:
	PMTEntry pmtEntries[TABLE_SIZE];
	unsigned head;
	unsigned fragment;

	friend class KernelSystem;
	friend class FirstLevelTable;
};

#endif