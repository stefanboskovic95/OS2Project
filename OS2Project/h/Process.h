#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "vm_declarations.h"

class KernelProcess;
class System;
class KernelSystem;

class Process {
public:

	Process(ProcessId pid);

	~Process();

	ProcessId getProcessId() const;
	
	Status createSegment(VirtualAddress startAddress, PageNum segmentSize, 
						AccessType type);
	Status loadSegment(VirtualAddress startAddress, PageNum segmentSize, 
					AccessType type, void* content);
	Status deleteSegment(VirtualAddress startAddress);

	Status pageFault(VirtualAddress startAddress);
	PhysicalAddress getPhysicalAddress(VirtualAddress address);
private:

	KernelProcess *pProcess;
	friend class System;
	friend class KernelSystem;
};

#endif