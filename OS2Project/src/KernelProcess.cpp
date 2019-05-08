#include "KernelProcess.h"
#include "KernelSystem.h"
#include "PMTEntry.h"
#include "FirstLevelTable.h"
#include "SecondLevelTable.h"
#include "part.h"
#include "SecondChance.h"

#define FIRST_LEVEL 128

std::mutex KernelProcess::createMutex;
std::mutex KernelProcess::pageFaultMutex;

KernelProcess::KernelProcess(ProcessId pid) {
	this->pid = pid;
	pmtp = nullptr;
	numberOfPages = 0;
	prev = next = nullptr;
	secondChance = nullptr;
}

KernelProcess::~KernelProcess() {
	if (prev) {
		prev->next = next;
	}
	else if (!prev) {
		kernelSystem->processListHead = next;
	}
	if (next) {
		next->prev = prev;
	}
	// Treba postaviti i pokazivac u Second Chance algoritmu na nullptr!
	if (secondChance) {
		secondChance->process = nullptr;
	}
}

ProcessId KernelProcess::getProcessId() {
	return pid;
}

Status KernelProcess::createSegment(VirtualAddress startAddress, PageNum segmentSize,
	AccessType type) {
	return loadSegment(startAddress, segmentSize, type, nullptr);
}

VirtualAddress alignToPage(VirtualAddress address) {
	return address / PAGE_SIZE * PAGE_SIZE;
}

/*
	Algoritam ubacivanja stranice:
	1. Provjeri da li se VA poklapa sa pocetkom stranice
	2. Alociraj tabelu prvog nivoa
	3. Alociraj jednu po jednu stranicu
	4.1 Ako nema mjesta za stranicu u OM, probaj da je alociras na disk
	4.2 Ako nema mjesta ni na disku, obrisi segment, vrati odgovarajuci status
	5. Ubacuj deskriptor jedne stranice uporedo sa alokacijom stranice
	5.0 Deskriptor se ubacuje na zadatu VA, ako u nekom trenutku krene da ga ubacuje
		u deskriptor koji pripada drugom segmentu - greska, obrisati segment
	5.1 Ako nema mjesta za deskriptor, obrisi segment i vrati odgovarajuci status
	6. Inicijalizuj sadrzaj ili u OM, ili na disku
*/
Status KernelProcess::loadSegment(VirtualAddress startAddress, PageNum segmentSize,
	AccessType type, void* content) {
	std::lock_guard<std::mutex> lock(createMutex);

	if (startAddress != alignToPage(startAddress)) {
		return TRAP;
	}

	// 2. Alociraj tabelu prvog nivoa
	if (pmtp == nullptr) {
		pmtp = kernelSystem->getFirstLevelTable();
	}

	// Nema prostora za pmt
	if (pmtp == nullptr) {
		return TRAP;
	}

	bool segmentStart = true;
	VirtualAddress currentPageVA = startAddress;
	PageNum pageCounter = 0;
	PageNum contentCounter = 0;
	
	do {
		// 3. Alocira se po jedna stranica
		// getPageSegment ce da vrati nullptr ako nema mjesta
		PhysicalAddress pageStartAddress = kernelSystem->getPageSegment(1);
		unsigned cluster = ~0;
		if (pageStartAddress == nullptr) {
			// 4.1 Ako nema mjesta za stranicu u OM, probaj da je alociras na disk
			Status status = kernelSystem->getCluster(cluster);
			// 4.2 Ako nema mjesta ni na disku, obrisi segment, vrati odgovarajuci status
			if (status == TRAP) {
				deleteSegment(startAddress);
				return TRAP;
			}
		}

		// 5. Ubacuj deskriptor jedne stranice uporedo sa alokacijom stranice
		// Ovo moze i treba u jednu f-ju
		unsigned frame;
		unsigned long processVMStartSpace;
		if (pageStartAddress != nullptr) {
			unsigned long segmentAddress = reinterpret_cast<unsigned long> (pageStartAddress);
			PhysicalAddress processVMSpace = kernelSystem->processVMSpace;
			processVMStartSpace = reinterpret_cast<unsigned long> (processVMSpace);
			frame = (segmentAddress - processVMStartSpace) / 1024;;
		}
		else {
			frame = cluster;
		}
		
		// accessed, segmentStart, valid, loaded, rwx, frame
		PMTEntry pmtEntry(0, 0, 1, 1, type, frame);
		numberOfPages++;

		// Ako je na disku
		if (pageStartAddress == nullptr) {
			pmtEntry.setLoaded(0);
		}

		// Pocetak segmenta
		if (segmentStart == true) {
			pmtEntry.setSegmentStart(1);
			segmentStart = false;
		}

		// 5.0 Deskriptor se ubacuje na zadatu VA, ako u nekom trenutku krene da ga ubacuje
		//     u deskriptor koji pripada drugom segmentu - greska, obrisati segment
		Status status = pmtp->addPMTEntry(pmtEntry, currentPageVA);
		currentPageVA += PAGE_SIZE;

		// 5.1 Ako nema mjesta za deskriptor, obrisi segment i vrati odgovarajuci status
		if (status == TRAP) {
			status = deleteSegment(startAddress);
			// Ako brisanje nije uspjelo - tabela drugog nivoa je nullptr
			// i treba "rucno" osloboditi, ili klaster ili stranicu
			if (status == TRAP) {
				if (pageStartAddress != nullptr) {
					kernelSystem->releasePageSegment(pageStartAddress, 1);
				}
				else {
					kernelSystem->addCluster(cluster, 1);
				}
			}
			return TRAP;
		}

		// 6. Inicijalizuj sadrzaj ili u OM, ili na disku
		if (content != nullptr) {
			unsigned long contentAddr = reinterpret_cast<unsigned long> (content) + contentCounter;
			PhysicalAddress contentPhyAddr = reinterpret_cast<PhysicalAddress> (contentAddr);
			char* charContent = reinterpret_cast<char*> (contentPhyAddr);
			// U OM
			if (pageStartAddress != nullptr) {
				unsigned long pageStartAddress = processVMStartSpace + frame;

				char* om = reinterpret_cast<char*> (pageStartAddress);

				for (unsigned i = 0; i < PAGE_SIZE; i++) {
					//om[i] = charContent[contentCounter++];
					om[i] = charContent[i++];
				}
				contentCounter += PAGE_SIZE;
			}
			// Na HDD
			else {
				kernelSystem->partition->writeCluster(cluster, charContent);
			}
		}

		pageCounter++;
	} while (pageCounter < segmentSize);
}

Status KernelProcess::deleteSegment(VirtualAddress startAddress) {
	PMTEntry* descriptor = getPMTEntry(startAddress);

	if (descriptor == nullptr || descriptor->getSegmentStart() == 0 || descriptor->getValid() == 0) {
		return TRAP;
	}

	VirtualAddress nextAddress = startAddress;
	unsigned size = 0;
	bool start = true;

	do {
		descriptor->setValid(0);

		// Ovo je sasvim ok, zato sto se preko tabela preslikavanja dobija deskriptor
		nextAddress += PAGE_SIZE;

		if (descriptor->getLoaded() == 1) {
			kernelSystem->addSegment(getPhyAddrByPage(descriptor->getFrame()), 1 * PAGE_SIZE);
		}
		else {
			kernelSystem->addCluster(descriptor->getFrame(), 1);
		}

		descriptor = getPMTEntry(nextAddress);
	} while (descriptor && descriptor->getStatus() != 0 && descriptor->getSegmentStart() != 1);
}

PhysicalAddress KernelProcess::getPhysicalAddress(VirtualAddress address) {
	PhysicalAddress ret = nullptr;

	PMTEntry* descriptor = getPMTEntry(address);

	if (descriptor == nullptr) {
		return nullptr;
	}

	if (descriptor->getValid() == 0) {
		return nullptr;
	}

	if (descriptor->getLoaded() == 0) {
		return nullptr;
	}

	PhysicalAddress processVMSpace = kernelSystem->processVMSpace;
	unsigned long pageAddress = reinterpret_cast<unsigned long> (processVMSpace);
	pageAddress += descriptor->getFrame();

	ret = reinterpret_cast<PhysicalAddress> (pageAddress);

	return ret;
}

Status KernelProcess::pageFault(VirtualAddress startAddress) {
	std::lock_guard<std::mutex> lock(pageFaultMutex);
	//std::lock_guard<std::mutex> lock(kernelSystem->accesMutex);

	PMTEntry* descriptor = getPMTEntry(startAddress);

	if (descriptor == nullptr) {
		return TRAP;
	}

	if (descriptor->getValid() == 0) {
		return TRAP;
	}

	PhysicalAddress pageStartAddress = nullptr;

	pageStartAddress = kernelSystem->getPageSegment(1);

	if (pageStartAddress == nullptr) {
		// Deskriptor i sadrzaj se azuriraju unutar getVictimPage, za victim page
		pageStartAddress = kernelSystem->getVictimPage();

		if (pageStartAddress == nullptr) {
			return TRAP;
		}
		kernelSystem->addCluster(descriptor->getFrame(), 1);

		// Dohvati sadrzaj sa diska i upisi u odgovarajucu stranicu (koja se ucitava)
		char buffer[PAGE_SIZE];
		kernelSystem->partition->readCluster(descriptor->getFrame(), buffer);
		char* om = reinterpret_cast<char*>(pageStartAddress);

		for (unsigned i = 0; i < PAGE_SIZE; i++) {
			om[i] = buffer[i];
		}
	}

	// Ovo se moze zapakovati u jednu funkciju!
	unsigned long segmentAddress = reinterpret_cast<unsigned long> (pageStartAddress);
	unsigned long processVMStartSpace = reinterpret_cast<unsigned long>(kernelSystem->processVMSpace);
	unsigned frame = (segmentAddress - processVMStartSpace) / 1024;
	
	// Auziriaj deskriptor stranice koja se ucitava u memoriju
	descriptor->setAccessed(0);
	descriptor->setFrame(frame);
	descriptor->setLoaded(1);

	return OK;
}

PhysicalAddress KernelProcess::getPhyAddrByPage(PageNum page) {
	PhysicalAddress processVMSpace = kernelSystem->processVMSpace;
	unsigned long pageAddress = reinterpret_cast<unsigned long> (processVMSpace);

	// Broj stranica * velicina stranice
	pageAddress += page * PAGE_SIZE;

	return reinterpret_cast<PhysicalAddress> (pageAddress);
}


PMTEntry* KernelProcess::getPMTEntry(VirtualAddress address) {
	if (pmtp == nullptr) {
		return nullptr;
	}

	PageNum firstTableIndex = (address >> (OFFSET + TABLE_INDEX));
	PageNum secondTableIndex = (address >> OFFSET) & TABLE_MASK;

	FirstLevelTable firstLevel = *pmtp;
	SecondLevelTable* secondLevel = firstLevel[firstTableIndex].getSecondLevel();

	if (secondLevel == nullptr) {
		return nullptr;
	}

	return &(*secondLevel)[secondTableIndex];
}

PageNum KernelProcess::getPage(VirtualAddress address) {
	PageNum page = (address >> OFFSET) & MAX_PAGE;
	return page;
}