#ifndef CACHE_FXNS_H
#define CACHE_FXNS_H
#include <iostream>
#include "memory_driver.h"
#include "cache.h"

Cache* initCache(int mode)
{
	Cache* res;
	switch (mode)
	{
	case DM://DM
		res = new DMCache();
		break;
	case SA://SA
		res = new SACache();
		break;
	case FA://FA
		res = new FACache();
		break;
	default:
		std::cout << "The mode provided to initialize cache is out of range.\n";
		exit(5);
		break;
	}
	//res->print();
	return res;
}
int cacheController(Cache* cache, int prev_status,bool MemR, bool MemW, 
	int& data, int adr, int& miss, int myMem[], int& hit_count)
{
	/*
	Same as your processor, the cache controller has a(n) (implicit) “clock” and an (infinite)
while loop. During each iteration, your controller checks the (previous) Status and does
the following based on this value:

		1. If it is 1, it checks control signals (i.e., MemR and MemW).
		If either is one, it calls
		the corresponding function (i.e., Load or Store).
		2. If it is zero, it outputs data=MM[adr] and Status=1.
		3. If it is negative, it increments the Status by one.*/
	int status = prev_status;
	if (prev_status == 1)
	{
		//std::cout << "Address: " << adr << "\n";
		if (MemR == 1)
		{
			std::cout << "Attempting to read ";
			std::cout << "at address " << adr;
			
			bool hit = cache->lw(adr, data, status, myMem);
			if (!hit)
				miss++;
			else
				hit_count++;
		}
		else if (MemW == 1)
		{
			std::cout << "Attempting to store "<<data<<" at address " << adr << ".";
			bool hit = cache->sw(adr, data, status, myMem);
			//if (!hit)
			//	miss++;
			if (hit)
				hit_count++;
		}
	}
	else if (prev_status == 0)
	{
		status = 1;
		data = myMem[adr];
	}
	else if (prev_status < 0)
	{
		status++;
	}
	else
	{
		std::cout << "The value of status was out of range!\n";
		std::cout << "Status: " << status;
		exit(10);
	}
	//cache->print();
	return status;
}
int memory_controller(bool cur_MemR, bool cur_MemW, int& cur_data, int cur_adr,
	int status, int& miss, std::string mode, Cache* cache, int myMem[], int& hit_count)
{
	//cache->print();
	return cacheController(cache, status, cur_MemR, cur_MemW, cur_data, cur_adr, miss, myMem, hit_count);
}
void printMainMem(int myMem[])
{
	std::cout << "Main Memory:\n";
	for (int i = 0; i < MEM_SIZE; i+=4)
	{
		std::cout << myMem[i]<<"  ";
		std::cout << myMem[i]<<"  ";
		std::cout << myMem[i]<<"  ";
		std::cout << myMem[i]<<"\n";
	}
}

#endif
