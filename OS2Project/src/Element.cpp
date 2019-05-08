#include "Element.h"

Element::Element(unsigned long size, Element* prev, Element* next) {
	this->size = size;
	this->prev = prev;
	this->next = next;
}

PhysicalAddress Element::getStartAddress() {
	PhysicalAddress startAddress = reinterpret_cast<PhysicalAddress> (this);
	return startAddress;
}