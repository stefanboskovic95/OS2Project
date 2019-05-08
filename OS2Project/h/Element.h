#ifndef _ELEMENT_H_
#define _ELEMENT_H_

#include "vm_declarations.h"

class FreeList;

class Element{
public:

	Element(unsigned long size, Element* prev, Element* next);

	PhysicalAddress getStartAddress();

private:
	Element* prev;
	Element* next;

	// Velicina slobodnog prostora u bajtovima!
	unsigned long size;

	friend class FreeList;
	friend class FreeListIterator;
};

#endif