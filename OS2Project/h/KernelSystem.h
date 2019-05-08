#ifndef _KERNELSYSTEM_H_
#define _KERNELSYSTEM_H_

#include "vm_declarations.h"

class Partition;
class Process;
class FreeList;
class FirstLevelTable;
class SecondLevelTable;
class KernelProcess;
class SecondChance;

#include <iostream>
#include <mutex>
#include <list>
using namespace std;

class KernelSystem {
public:

	KernelSystem(PhysicalAddress processVMSpace, PageNum processVMSpaceSize,
		PhysicalAddress pmtSpace, PageNum pmtSpaceSize,
		Partition* partition);

	~KernelSystem();

	Process* createProcess();

	Time periodicJob();

	// Hardware job
	Status access(ProcessId pid, VirtualAddress address, AccessType type);

	// Alokacija stranica unutar zadatog prostora
	PhysicalAddress getPageSegment(PageNum segmentSize);

	// Vise se ne poziva
	PhysicalAddress getPmtSpace(PageNum segmentSize, unsigned long sizeofEntry);

	FirstLevelTable* getFirstLevelTable();
	void releasePageSegment(PhysicalAddress pageStartAddress, PageNum segmentSize);

	SecondLevelTable* getSecondLevelTable();
	void releaseFirstLevelTable(FirstLevelTable* firstLevelTable);

	PhysicalAddress getVictimPage();

	friend ostream& operator<<(ostream& os, const KernelSystem& system);

	static KernelProcess* getProcessById(ProcessId pid);

	static KernelProcess* getNextProcess(ProcessId pid);

	static PageNum getFirstLevelTableIndex(VirtualAddress va);
	static PageNum getSecondLevelTableIndex(VirtualAddress va);

private:

	void addSegment(PhysicalAddress startAddress, unsigned size);

	PhysicalAddress processVMSpace;
	PageNum processVMSpaceSize;
	PhysicalAddress pmtSpace;
	PageNum pmtSpaceSize;
	Partition* partition;

	SecondChance* secondChance;

	static ProcessId lastProcessId;

	friend class Process;
	friend class KernelProcess;

	FreeList* freePmtSpace;
	FreeList* freePageSpace;

	static KernelProcess* processListHead;
	static void addToProcessList(KernelProcess* process);

	std::list<std::tuple<unsigned, unsigned>> clusterList;
	Status getCluster(unsigned& cluster);
	void addCluster(unsigned clusterNum, unsigned size);

	std::mutex createProcessMutex;
	std::mutex accesMutex;
};


#endif