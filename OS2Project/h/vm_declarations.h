#ifndef _VM_DECLARATIONS_H_
#define _VM_DECLARATIONS_H_

typedef unsigned long PageNum;
typedef unsigned long VirtualAddress;
typedef void* PhysicalAddress;
typedef unsigned long Time;

enum Status {OK, PAGE_FAULT, TRAP};

enum AccessType { READ, WRITE, READ_WRITE, EXECUTE };

typedef unsigned ProcessId;

#define PAGE_SIZE 1024

// 2^24 - 1 
#define MAX_VA_ADDRESS 16777215
#define MAX_PAGE 16383
#define MAX_OFFSET 1023
#define OFFSET 10

#define TABLE_SIZE 128


#define TABLE_INDEX 7
#define TABLE_MASK 127

#endif