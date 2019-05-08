#ifndef _FIRST_LEVEL_TABLE_H_
#define _FIRST_LEVEL_TABLE_H_

#include "vm_declarations.h"
#include "FirstLevelEntry.h"

class SecondLevelTable;
class KernelSystem;
class PMTEntry;

class FirstLevelTable {
public:
	FirstLevelTable(KernelSystem* kernelSystem);

	FirstLevelEntry& operator[](unsigned index);

	Status addPMTEntry(const PMTEntry& descriptor, VirtualAddress descriptorAddress);

private:
	FirstLevelEntry secondLevelTables[TABLE_SIZE];
	unsigned head;
	unsigned fragment;

	KernelSystem* kernelSystem;

	friend class KernelSystem;
};

#endif