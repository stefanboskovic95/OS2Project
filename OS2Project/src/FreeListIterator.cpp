#include "FreeListIterator.h"
#include "FreeList.h"
#include "Element.h"

FreeListIterator::FreeListIterator(FreeList freeList) {
	this->head = freeList.head;
}

Element* FreeListIterator::nextElement() {
	if (!head) {
		return nullptr;
	}
	return head = head->next;
}

unsigned FreeListIterator::getCurrentElementSize() {
	if (!head) {
		return 0;
	}
	return head->size;
}