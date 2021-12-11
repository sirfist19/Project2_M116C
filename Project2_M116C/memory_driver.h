#ifndef MEMORY_DRIVER_H
#define MEMORY_DRIVER_H

#define CACHE_UNITS 16
#define MEM_SIZE 2048
#define CACHE_WAYS 1

#define BLOCK_SIZE 1 // bytes per block
#define DM 0
#define FA 1
#define SA 2

struct cache_unit
{
	int tag; // you need to compute offset and index to find the tag.
	int lru_position; // for FA only
	int data; // the actual data stored in the cache/memory
	// add more things here if needed
};

struct trace
{
	bool MemR;
	bool MemW;
	int adr;
	int data;
};
#endif
