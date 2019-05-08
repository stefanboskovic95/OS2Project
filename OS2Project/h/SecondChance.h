#ifndef _SECOND_CHANCE_H_
#define _SECOND_CHANCE_H_

#include "vm_declarations.h"

class KernelProcess;
class PMTEntry;

class SecondChance {
public:

	SecondChance();

	PMTEntry* getVicitmEntry();

private:
	KernelProcess* process;
	PageNum clockHand;

	friend class KernelProcess;
};

#endif