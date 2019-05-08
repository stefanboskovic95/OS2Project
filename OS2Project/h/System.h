#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "vm_declarations.h"

class Partition;
class KernelProcess;
class KernelSystem;
class Process;

class System {
public:

	System(PhysicalAddress processVMSpace, PageNum processVMSpaceSize,
		PhysicalAddress pmtSpace, PageNum pmtSpaceSize,
		Partition* partition);

	~System();

	Process* createProcess();

	Time periodicJob();

	// Hardware job
	Status access(ProcessId pid, VirtualAddress address, AccessType type);
private:

	KernelSystem *pSystem;
	friend class Process;
	friend class KernelProcess;
};

#endif