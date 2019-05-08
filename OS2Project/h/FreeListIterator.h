#ifndef _FREE_LIST_ITERRATOR_H
#define _FREE_LIST_ITERRATOR_H

class FreeList;
class Element;

class FreeListIterator {
public:
	FreeListIterator(FreeList freeList);

	Element* nextElement();
	unsigned getCurrentElementSize();

private:
	Element* head;
};

#endif