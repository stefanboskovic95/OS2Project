#ifndef _FREELIST_H_
#define _FREELIST_H_

#include "vm_declarations.h"
#include <iostream>
using namespace std;

class Element;

/*
	Napomena:
	FreeList cuva slobodan prostor u bajtovima!
*/
class FreeList{
public:
	FreeList();

	void addSegment(PhysicalAddress startAddress, unsigned long size);

	PhysicalAddress getSegment(unsigned long size, unsigned& fragmentSize);

	PhysicalAddress removeSegment(unsigned long size, unsigned& fragmentSize);

	bool isThereSpace(unsigned long size);

	friend ostream& operator<<(ostream& os, const FreeList& l);

private:

	Element* head;

	Element* getElement(unsigned long size);
	void detachElement(Element* elem);

	Element* getElementByEndAddress(PhysicalAddress endAddress);
	Element* getElementByStartAddress(PhysicalAddress startAddress);


	friend class FreeListIterator;
};

#endif