#include "FirstLevelEntry.h"

FirstLevelEntry::FirstLevelEntry() {
	secondLevelTable = nullptr;
}

FirstLevelEntry::FirstLevelEntry(SecondLevelTable* secondLevelTable) {
	this->secondLevelTable = secondLevelTable;
}

SecondLevelTable* FirstLevelEntry::getSecondLevel() {
	return secondLevelTable;
}

void FirstLevelEntry::setSecondLevel(SecondLevelTable* secondLevelTable) {
	this->secondLevelTable = secondLevelTable;
}