#include "SecondChance.h"
#include "KernelSystem.h"
#include "PmtEntry.h"
#include "KernelProcess.h"
#include "FirstLevelTable.h"
#include "SecondLevelTable.h"

SecondChance::SecondChance() {
	clockHand = 0;
	process = nullptr;
}

PMTEntry* SecondChance::getVicitmEntry() {
	PMTEntry* ret = nullptr;

	// Inicijalizacija, ne moze u konstruktoru zato sto je 
	// SecondChance staticko polje u klasi system
	if (process == nullptr) {
		process = KernelSystem::getNextProcess(~0);

		process->secondChance = this;
	}

	while (clockHand < process->numberOfPages) {
		PageNum firstTableIndex = clockHand / TABLE_SIZE;
		PageNum secondTableIndex = clockHand % TABLE_SIZE;

		FirstLevelTable firstLevel = *(process->pmtp);
		SecondLevelTable* secondLevel = firstLevel[firstTableIndex].getSecondLevel();

		if (secondLevel == nullptr) {
			return nullptr;
		}

		clockHand++;

		if (clockHand == process->numberOfPages) {
			process->secondChance = nullptr;

			process = KernelSystem::getNextProcess(process->pid);
			clockHand = 0;
			process->secondChance = this;
		}


		PMTEntry* pmt = &(*secondLevel)[secondTableIndex];
		if (pmt->getStatus() == 0) {
			pmt = &(*secondLevel)[++secondTableIndex];
		}

		if (pmt->getLoaded() != 0) {
			if (pmt->getAccessed() == 0) {
				ret = pmt;
				break;
			}
			else {
				pmt->setAccessed(0);
			}
		}
	}

	return ret;
}