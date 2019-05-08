#include "Process.h"
#include "KernelProcess.h"

Process::Process(ProcessId pid) {
	pProcess = new KernelProcess(pid);
}

Process::~Process() {
	delete pProcess;
}

ProcessId Process::getProcessId() const {
	return pProcess->getProcessId();
}

Status Process::createSegment(VirtualAddress startAddress, PageNum segmentSize,
							AccessType type) {
	return pProcess->createSegment(startAddress, segmentSize, type);
}

Status Process::loadSegment(VirtualAddress startAddress, PageNum segmentSize,
						AccessType type, void* content) {
	return pProcess->loadSegment(startAddress, segmentSize, type, content);
}

Status Process::deleteSegment(VirtualAddress startAddress) {
	return pProcess->deleteSegment(startAddress);
}

Status Process::pageFault(VirtualAddress startAddress) {
	return pProcess->pageFault(startAddress);
}

PhysicalAddress Process::getPhysicalAddress(VirtualAddress address) {
	return pProcess->getPhysicalAddress(address);
}