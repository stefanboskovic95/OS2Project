#ifndef _KERNELPROCESS_H_
#define _KERNELPROCESS_H_

#include "vm_declarations.h"
#include<list>
#include<mutex>

class Process;
class PMTEntry;
class FirstLevelTable;
class KernelSystem;

class KernelProcess {
public:

	KernelProcess(ProcessId pid);

	~KernelProcess();

	ProcessId getProcessId();
	
	Status createSegment(VirtualAddress startAddress, PageNum segmentSize,
						AccessType type);
	Status loadSegment(VirtualAddress startAddress, PageNum segmentSize,
					AccessType type, void* content);
	Status deleteSegment(VirtualAddress startAddress);

	Status pageFault(VirtualAddress startAddress);
	PhysicalAddress getPhysicalAddress(VirtualAddress address);

private:
	
	PageNum getPage(VirtualAddress address);
	PMTEntry* getPMTEntry(VirtualAddress address);
	PhysicalAddress getPhyAddrByPage(PageNum page);

	FirstLevelTable *pmtp;
	PageNum numberOfPages;

	KernelSystem* kernelSystem;

	ProcessId pid;

	friend class KernelSystem;
	friend class SecondChance;

	KernelProcess* next;
	KernelProcess* prev;

	// Ovdje jeste static, za razliku od KernelSystem
	static std::mutex createMutex;
	static std::mutex pageFaultMutex;

	SecondChance* secondChance;
};

#endif