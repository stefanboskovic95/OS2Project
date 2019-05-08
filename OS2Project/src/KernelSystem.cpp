#include "KernelSystem.h"
#include "KernelProcess.h"
#include "Process.h"
#include "PMTEntry.h"
#include "FirstLevelEntry.h"
#include "FreeList.h"
#include "FirstLevelTable.h"
#include "SecondLevelTable.h"
#include "SecondChance.h"
#include "part.h"

ProcessId KernelSystem::lastProcessId = 0;
KernelProcess* KernelSystem::processListHead;

KernelSystem::KernelSystem(PhysicalAddress processVMSpace, PageNum processVMSpaceSize,
		PhysicalAddress pmtSpace, PageNum pmtSpaceSize,
		Partition* partition) {
	this->processVMSpace = processVMSpace;
	this->processVMSpaceSize = processVMSpaceSize;
	this->pmtSpace = pmtSpace;
	this->pmtSpaceSize = pmtSpaceSize;
	this->partition = partition;

	secondChance = new SecondChance();

	freePageSpace = new FreeList();
	freePmtSpace = new FreeList();

	// Citav fizicki prostor i citav pmt prostor su slobodni
	freePageSpace->addSegment(processVMSpace, PAGE_SIZE * processVMSpaceSize);
	freePmtSpace->addSegment(pmtSpace, PAGE_SIZE * pmtSpaceSize);

	// Slobodan je i sav prostor na disku
	clusterList.emplace_back(0, partition->getNumOfClusters());
}

KernelSystem::~KernelSystem() {
	delete secondChance;
	delete freePageSpace;
	delete freePmtSpace;
	clusterList.clear();
}

Process* KernelSystem::createProcess() {
	std::lock_guard<std::mutex> lock(createProcessMutex);

	ProcessId pid = lastProcessId++;
	Process* process = new Process(pid);
	process->pProcess->kernelSystem = this;
	addToProcessList(process->pProcess);
	return process;
}

Time KernelSystem::periodicJob() {
	return 0;
}

Status KernelSystem::access(ProcessId pid, VirtualAddress address, AccessType type) {
	std::lock_guard<std::mutex> lock(accesMutex);

	KernelProcess *process = getProcessById(pid);

	PageNum firstTableIndex = getFirstLevelTableIndex(address);
	PageNum secondTableIndex = getSecondLevelTableIndex(address);

	FirstLevelTable firstLevel = *process->pmtp;
	SecondLevelTable* secondLevel = firstLevel[firstTableIndex].getSecondLevel();

	if (secondLevel == nullptr) {
		return TRAP;
	}

	PMTEntry* descriptor = &(*secondLevel)[secondTableIndex];

	if (descriptor->getValid() == 0) {
		return TRAP;
	}

	if (descriptor->getAcessType() == READ_WRITE) {
		if (type != READ && type != WRITE) {
			return TRAP;
		}
	}
	else if (type != descriptor->getAcessType()) {
		return TRAP;
	}

	if (descriptor->getLoaded() == 0) {
		return PAGE_FAULT;
	}
	else {
		descriptor->setAccessed(1);
	}

	return OK;
}

PhysicalAddress KernelSystem::getPageSegment(PageNum segmentSize) {
	unsigned fragment = 0;

	// FreeList cuva u bajtovima
	PhysicalAddress ret = freePageSpace->removeSegment(segmentSize * PAGE_SIZE, fragment);

	return ret;
}

PhysicalAddress KernelSystem::getPmtSpace(PageNum segmentSize, unsigned long sizeOfEntry) {
	unsigned fragment = 0;

	PhysicalAddress ret = freePmtSpace->removeSegment(segmentSize * sizeOfEntry, fragment);

	return ret;
}

FirstLevelTable* KernelSystem::getFirstLevelTable() {
	FirstLevelTable firstLevelTable(this);

	//unsigned size = TABLE_SIZE * sizeof(FirstLevelTable);
	unsigned size = sizeof(FirstLevelTable);
	unsigned fragment = 0;

	PhysicalAddress startAddress = freePmtSpace->removeSegment(size, fragment);

	if (startAddress == nullptr) {
		return nullptr;
	}

	FirstLevelTable* ret = reinterpret_cast<FirstLevelTable*> (startAddress);
	ret->fragment = fragment;
	ret[0] = firstLevelTable;

	return ret;
}

SecondLevelTable* KernelSystem::getSecondLevelTable() {
	SecondLevelTable secondLevelTable;

	//unsigned long sizeOfSecondLevelTable = TABLE_SIZE*sizeof(PMTEntry);
	unsigned long sizeOfSecondLevelTable = sizeof(SecondLevelTable);
	unsigned fragment = 0;

	PhysicalAddress startAddress = freePmtSpace->removeSegment(sizeOfSecondLevelTable, fragment);

	if (startAddress == nullptr) {
		return nullptr;
	}

	SecondLevelTable* ret = reinterpret_cast<SecondLevelTable*> (startAddress);
	ret->fragment = fragment;
	ret[0] = secondLevelTable;

	return ret;
}

void KernelSystem::releasePageSegment(PhysicalAddress pageStartAddress, PageNum segmentSize) {
	// Napomena: FreeList cuva prostor u bajtovima!!!
	freePageSpace->addSegment(pageStartAddress, segmentSize * PAGE_SIZE);
}


void KernelSystem::releaseFirstLevelTable(FirstLevelTable* firstLevelTable) {
	PhysicalAddress startAddress = reinterpret_cast<PhysicalAddress> (firstLevelTable);
	freePmtSpace->addSegment(startAddress, sizeof(FirstLevelTable));
}

void KernelSystem::addSegment(PhysicalAddress startAddress, unsigned size) {
	freePageSpace->addSegment(startAddress, size);
}

std::ostream& operator<<(std::ostream& os, const KernelSystem& system){
	os << "Page space: " << (*system.freePageSpace) << std::endl;
	os << "Pmt space: " << (*system.freePmtSpace) << std::endl;
	os << "Disk space: " << std::endl;
	for (auto iter = system.clusterList.begin(); iter != system.clusterList.end(); iter++) {
		os << "cluster: " << std::get<0>(*iter) << ", size: " << std::get<1>(*iter) << std::endl;
	}
	return os;
}



// Second chance algorithm
PhysicalAddress KernelSystem::getVictimPage() {
	PhysicalAddress ret = nullptr;

	PMTEntry *pmt = secondChance->getVicitmEntry();

	if (pmt == nullptr) {
		return nullptr;
	}

	// Dohvati klaster na koji ce da bude izbacena stranica
	unsigned cluster = ~0;
	Status status = getCluster(cluster);

	// Nema mjesta na disku
	if (status == TRAP) {
		return nullptr;
	}

	unsigned long address = reinterpret_cast<unsigned long> (processVMSpace);
	address += pmt->getFrame() * 1024;

	// Azuriraj deskriptor stranice koja se izbacuje
	pmt->setFrame(cluster);
	pmt->setLoaded(0);
	
	
	ret = reinterpret_cast<PhysicalAddress> (address);

	// Sacuvaj sadrzaj na disku
	char* content = reinterpret_cast<char*> (ret);
	partition->writeCluster(cluster, content);

	return ret;
}

void KernelSystem::addToProcessList(KernelProcess* process) {
	if (!processListHead) {
		processListHead = process;
	}
	else {
		// Ubaci na pocetak liste
		KernelProcess* head = processListHead;
		// head->next je ili nullptr, ili je postavljeno u prethodnom pozivu f-je
		head->prev = process;
		process->next = head;
		processListHead = process;
	}
}

KernelProcess* KernelSystem::getProcessById(ProcessId pid) {
	KernelProcess* process = processListHead;
	while (process != nullptr) {
		if (process->pid == pid) {
			return process;
		}
		process = process->next;
	}
	return nullptr;
}

KernelProcess* KernelSystem::getNextProcess(ProcessId pid) {
	KernelProcess* process = processListHead;

	if (processListHead == nullptr) {
		return nullptr;
	}

	while (process != nullptr) {
		// < jer se procesi ubacuju na pocetak liste, 
		// a proces dobija najveci broj prilikom kreiranja
		if (process->pid < pid) {
			return process;
		}
		process = process->next;
	}

	// Ako je prosao kroz citavu listu, kreni od pocetka
	if (process == nullptr) {
		return processListHead;
	}

	return nullptr;
}


PageNum KernelSystem::getFirstLevelTableIndex(VirtualAddress va) {
	PageNum firstTableIndex = (va >> (OFFSET + TABLE_INDEX));
	return firstTableIndex;
}

PageNum KernelSystem::getSecondLevelTableIndex(VirtualAddress va) {
	PageNum secondTableIndex = (va >> OFFSET) & TABLE_MASK;
	return secondTableIndex;
}

Status KernelSystem::getCluster(unsigned& cluster) {
	auto iter = clusterList.begin();
	
	for (; iter != clusterList.end(); iter++) {
		unsigned clusterSize = std::get<1>(*iter);
		cluster = std::get<0>(*iter);

		clusterList.remove(*iter);

		if (clusterSize > 1) {
			clusterList.emplace_back(cluster + 1, clusterSize - 1);
		}
		
		return OK;
	}

	return TRAP;
}

void KernelSystem::addCluster(unsigned clusterNum, unsigned size) {
	clusterList.emplace_front(clusterNum, size);
}