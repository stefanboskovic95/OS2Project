#ifndef _PMT_ENTRY_H_
#define _PMT_ENTRY_H_

#include<cstdint>
#include "vm_declarations.h"
#include<iostream>

#define RWX 3
#define LOADED 4
#define VALID 8
#define SEGMENT_START 16
#define ACCESSED 32

// 1111 1100
#define RWX_MASK 252
// 1111 1011
#define LOADED_MASK 251
// 1111 0111
#define VALID_MASK 247
// 1110 1111
#define SEGMENT_START_MASK 239
// 1101 1111
#define ACCESSED_MASK 223



class PMTEntry {
public:

	PMTEntry();

	~PMTEntry();

	PMTEntry(uint8_t accessed, uint8_t segmentStart, uint8_t valid,
		uint8_t loaded, AccessType rwx, uint16_t frame);

	AccessType getAcessType() const;

	unsigned getLoaded() const;

	unsigned getValid() const;

	unsigned getStatus() const;

	unsigned getSegmentStart() const;

	unsigned getAccessed() const;

	void setAccessed(uint8_t accessed);

	void setSegmentStart(uint8_t segmentStart);

	void setValid(uint8_t valid);

	void setLoaded(uint8_t loaded);

	void setFrame(unsigned frame);

	unsigned getFrame();

	friend std::ostream& operator<<(std::ostream& os, const PMTEntry& pmt);

private:
	// accessed, segmentStart, valid, loaded, rwx, frame
	uint8_t status; 
	// Za pristup VA prostoru: 10 000 Stranica od po 1024 B (char-ova)
	uint16_t frame;
};

#endif 