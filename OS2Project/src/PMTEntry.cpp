#include "PMTEntry.h"
#include <iostream>
using namespace std;

PMTEntry::PMTEntry() {
	status = 0;
	frame = 0;
}

PMTEntry::~PMTEntry() {
	status = 0;
	frame = 0;
}

PMTEntry::PMTEntry(uint8_t accessed, uint8_t segmentStart, uint8_t valid,
	uint8_t loaded, AccessType rwx, uint16_t frame) {
	status = accessed;
	status <<= 1;
	status |= segmentStart;
	status <<= 1;
	status |= valid;
	status <<= 1;
	status |= loaded;
	status <<= 2;
	status |= static_cast<uint8_t> (rwx);
	this->frame = frame;
}

AccessType PMTEntry::getAcessType() const {
	AccessType rwx = static_cast<AccessType> (status & RWX);
	return rwx;
}

unsigned PMTEntry::getLoaded() const {
	unsigned loaded = static_cast<unsigned> (status & LOADED);
	loaded = loaded >> 2;
	return loaded;
}

unsigned PMTEntry::getValid() const {
	unsigned valid = static_cast<unsigned> (status & VALID);
	valid = valid >> 3;
	return valid;
}

unsigned PMTEntry::getStatus() const {
	return status;
}

unsigned PMTEntry::getSegmentStart() const {
	unsigned segmentStart = static_cast<unsigned> (status & SEGMENT_START);
	segmentStart = segmentStart >> 4;
	return segmentStart;
}

unsigned PMTEntry::getAccessed() const {
	unsigned accessed = static_cast<unsigned> (status & ACCESSED);
	accessed >>= 5;
	return accessed;
}

void PMTEntry::setAccessed(uint8_t accessed) {
	accessed <<= 5;
	status &= ACCESSED_MASK;
	status |= accessed;
}

void PMTEntry::setSegmentStart(uint8_t segmentStart) {
	segmentStart <<= 4;
	status &= SEGMENT_START_MASK;
	status |= segmentStart;
}

void PMTEntry::setValid(uint8_t valid) {
	valid <<= 3;
	status &= VALID_MASK;
	status |= valid;
}

void PMTEntry::setLoaded(uint8_t loaded) {
	loaded <<= 2;
	status &= LOADED_MASK;
	status |= loaded;
}

void PMTEntry::setFrame(unsigned frame) {
	this->frame = static_cast<uint16_t> (frame);
}

unsigned PMTEntry::getFrame() {
	return frame;
}

ostream& operator<<(ostream& os, const PMTEntry& pmt) {
	os << "Status: " << pmt.getStatus() << endl;
	os << "Accessed: " << pmt.getAccessed() 
		<< " SegmentStart: " << pmt.getSegmentStart() 
		<< " Valid: " << pmt.getValid()
		<< " Loaded: " << pmt.getLoaded() 
		<< " RWX: " << pmt.getAcessType() 
		<< endl;
	return os;
}