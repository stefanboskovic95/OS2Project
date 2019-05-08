#ifndef _FIRST_LEVEL_ENTRY_H_
#define _FIRST_LEVEL_ENTRY_H_

#include "vm_declarations.h"

class SecondLevelTable;

class FirstLevelEntry{
public:
	FirstLevelEntry();

	FirstLevelEntry(SecondLevelTable* secondLevelTable);

	SecondLevelTable* getSecondLevel();

	void setSecondLevel(SecondLevelTable* secondLevelTable);

private:
	SecondLevelTable* secondLevelTable;

	friend class FirstLevelTable;
};

#endif