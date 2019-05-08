#include "FirstLevelTable.h"
#include "SecondLevelTable.h"
#include "PmtEntry.h"
#include "KernelSystem.h"

FirstLevelTable::FirstLevelTable(KernelSystem* kernelSystem) {
	this->kernelSystem = kernelSystem;
	head = 0;
	fragment = 0;
}

FirstLevelEntry& FirstLevelTable::operator[](unsigned index) {
	return secondLevelTables[index];
}

Status FirstLevelTable::addPMTEntry(const PMTEntry& descriptor, VirtualAddress descriptorAddress) {
	// Sve u jednu f-ju
	PageNum firstTableIndex = KernelSystem::getFirstLevelTableIndex(descriptorAddress);
	PageNum secondTableIndex = KernelSystem::getSecondLevelTableIndex(descriptorAddress);

	SecondLevelTable* secondLevelTable = secondLevelTables[firstTableIndex].getSecondLevel();

	if (secondLevelTable == nullptr) {
		SecondLevelTable* newTable = kernelSystem->getSecondLevelTable();
		if (newTable == nullptr) {
			return TRAP;
		}
		secondLevelTables[firstTableIndex].setSecondLevel(newTable);
		secondLevelTable = newTable;
	}

	PMTEntry* embeded = &(*secondLevelTable)[secondTableIndex];

	// Preklapa se sa postojecim segmentom
	if (embeded && embeded->getValid() == 1) {
		return TRAP;
	}

	*embeded = descriptor;
}