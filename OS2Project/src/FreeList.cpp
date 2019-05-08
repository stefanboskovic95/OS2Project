#include "FreeList.h"
#include "Element.h"
#include "FreeListIterator.h"


FreeList::FreeList() {
	head = nullptr;
}

void FreeList::addSegment(PhysicalAddress startAddress, unsigned long size) {
	PhysicalAddress fragmentAddress = startAddress;
	unsigned long fragmentSize = size;

	// Spoji sa slobodnim okolnim prostorom ako postoji
	if (head != nullptr) {
		Element* predecessor = getElementByEndAddress(fragmentAddress);
		unsigned long endAddr = reinterpret_cast<unsigned long> (fragmentAddress)+size;
		Element* successor = getElementByStartAddress(reinterpret_cast<PhysicalAddress> (endAddr));

		if (predecessor != nullptr) {
			fragmentAddress = predecessor->getStartAddress();
			fragmentSize += predecessor->size;
			detachElement(predecessor);
		}

		if (successor != nullptr) {
			fragmentSize += successor->size;
			detachElement(successor);
		}
	}
	Element* piece = reinterpret_cast<Element*> (fragmentAddress);

	// Ubacivanje na pocetak liste
	piece[0] = Element(fragmentSize, nullptr, head);


	if (head != nullptr) {
		head->prev = &piece[0];
		head = &piece[0];
	}
	else if (head == nullptr) {
		head = &piece[0];
	}
}

Element* FreeList::getElement(unsigned long size) {
	Element* iter = head;

	while (iter != nullptr) {
		if (iter->size >= size)
			break;
		iter = iter->next;
	}

	return iter;
}

PhysicalAddress FreeList::getSegment(unsigned long size, unsigned& fragmentSize) {
	Element* elem = getElement(size);

	if (elem == nullptr) {
		return nullptr;
	}

	if (elem->size > size) {
		if (elem->size < size + sizeof(Element)) {
			fragmentSize = elem->size - size;
		}
	}

	PhysicalAddress ret = reinterpret_cast<PhysicalAddress> (elem);

	return ret;
}

void FreeList::detachElement(Element* elem) {
	PhysicalAddress elemAddress = reinterpret_cast<PhysicalAddress> (elem);
	if (elem->prev) {
		elem->prev->next = elem->next;
	}
	if (elem->next) {
		elem->next->prev = elem->prev;
	}
	if (elem->next == nullptr && elem->prev == nullptr) {
		head = nullptr;
	}
	else if (head->getStartAddress() == elemAddress) {
		head = head->next;
	}
}

// Preko bocnih efekata vratiti ako je ostao mali fragment memorije, njegovu velicinu
PhysicalAddress FreeList::removeSegment(unsigned long size, unsigned& fragmentSize) {
	Element* elem = getElement(size);

	if (elem == nullptr) {
		return nullptr;
	}

	PhysicalAddress pa = reinterpret_cast<PhysicalAddress> (elem);
	unsigned long startAddress = reinterpret_cast<unsigned long> (pa);


	if (elem->size > size) {
		// Za previse male dijelove slobodnog prostora nije opravdano, niti je moguce voditi evidenciju!
		if (elem->size < size + sizeof(Element)) {
			fragmentSize = elem->size - size;
		}
		else {
			PhysicalAddress endAddress = reinterpret_cast<PhysicalAddress> (startAddress + size);
			// Prvo izbaciti elem iz liste!
			// pa onda dodati segment sa ostatkom memorije
			unsigned long elemSize = elem->size;
			detachElement(elem);
			addSegment(endAddress, elemSize - size);
		}
	}
	else if (elem->size == size) {
		PhysicalAddress endAddress = reinterpret_cast<PhysicalAddress> (startAddress + size);
		detachElement(elem);
	}
	else {
		return nullptr;
	}

	return pa;
}

bool FreeList::isThereSpace(unsigned long size) {
	Element* elem = getElement(size);

	if (elem == nullptr) {
		return false;
	}

	return true;
}

ostream& operator<<(ostream& os, const FreeList& l){
	FreeListIterator iter(l);

	do {
		unsigned size = iter.getCurrentElementSize();
		if (size == 0)
			break;
		os << size << " ";
	} while (iter.nextElement() != nullptr);

	return os;
}

Element* FreeList::getElementByEndAddress(PhysicalAddress endAddress) {
	Element* iter = head;

	while (iter != nullptr) {
		PhysicalAddress startAddress = reinterpret_cast<PhysicalAddress> (iter);
		unsigned long endAddr = reinterpret_cast<unsigned long> (startAddress)+iter->size;
		startAddress = reinterpret_cast<PhysicalAddress> (endAddr);

		if (startAddress == endAddress)
			break;
		iter = iter->next;
	}

	return iter;
}

Element* FreeList::getElementByStartAddress(PhysicalAddress startAddress) {
	Element* iter = head;

	while (iter != nullptr) {
		PhysicalAddress startAddr = reinterpret_cast<PhysicalAddress> (iter);

		if (startAddress == startAddr)
			break;
		iter = iter->next;
	}

	return iter;
}