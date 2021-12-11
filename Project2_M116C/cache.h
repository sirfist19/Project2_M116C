#ifndef CACHE_H
#define CACHE_H
#include <string>
#include <iostream>
#include <vector>
#include "memory_driver.h"

typedef std::vector<cache_unit> cache_set;
enum cacheAction
{
	load, store
};


class Cache//base class
{
protected:
	std::vector<cache_set> cache;//pointer to the dynamic array
public:
	Cache(std::vector<cache_set> content)
	{
		cache = content;
	}
	
	virtual bool lw(int& adr, int& data, int& status, int myMem[])
	{
		bool hit = search(adr, data, cacheAction::load);
		std::cout << "Hit: " << hit<<"\n";
		std::cout << "________________________________________\n";
		if (hit)
			status = 1;
		else//a miss
		{
			status = -3;
			//call 
			cache_miss(adr, data, myMem);
			//CacheMiss calls Evict
		}
		return hit;
	}
	void cache_miss(int adr, int data, int myMem[])
	{
		int value = myMem[adr];
		evict(adr, value);
	}
	virtual void evict(int adr, int data) = 0;
	virtual bool search(int& adr, int& data, cacheAction action) = 0;
	virtual void update_lru(int index, int tag) = 0;
	virtual bool sw(int& adr, int& data, int& status, int myMem[])
	{
		bool hit = search(adr, data, cacheAction::store);
		std::cout << "Hit: " << hit<<"\n";
		std::cout << "________________________________________\n";
		//update main memory
		myMem[adr] = data;
		status = 1;
		data = 0;
		return hit;
	}
	void print()
	{
		for (int i = 0; i < cache.size(); i++)
		{
			for (int j = 0; j < cache[i].size(); j++)
			{
				std::cout << "[" << i << ", " << j << "] ";
				std::cout << "Data: "<<cache[i][j].data<<" ";
				std::cout << "Tag: " << cache[i][j].tag << " ";
				std::cout << "LRU: " << cache[i][j].lru_position << "\n";
			}
		}
	}
};

class DMCache: public Cache
{
private:
	//cache_set cache[AMT_SETS_DM];

	std::vector<cache_set> setInitAndGetContent()
	{
		cache_unit origin;
		origin.tag = -1;// -1 indicates that the tag value is invalid. We don't use a separate VALID bit. 
		origin.lru_position = 0; // 0 means lowest position
		origin.data = 0;

		//for DM
		const int AMT_SETS_DM = CACHE_UNITS;
		std::vector<cache_set> cache(AMT_SETS_DM);

		for (int i = 0; i < AMT_SETS_DM; i++)
		{
			cache[i].push_back(origin);
		}
		return cache;
	}
public:
	DMCache() 
		:Cache(setInitAndGetContent())
	{
	}
	void update_lru(int index, int tag) {
		//nothing to update fro DM
		cache[index][0].lru_position = 0;
	}
	void evict(int adr, int data) 
	{
		int index = adr & 0xf;
		int tag = (adr & 0xfffffff0) >> 4;
		//first find candidate for eviction
		//update LRU counters
		//update the actual value and tag
		cache[index][0].data = data;
		cache[index][0].tag = tag;
		update_lru(index, tag);
	}
	bool search(int& adr, int& data, cacheAction action) 
	{
		//std::cout << "Searching!\n";
		int index = adr & 0xf;
		int tag = (adr & 0xfffffff0) >> 4;
		std::cout << " At index " << index<<"";
		std::cout << " with tag " << tag<<"\n";

		if (cache[index][0].tag == tag)//a hit!
		{
			if (action == cacheAction::load)
			{
				data = cache[index][0].data;//loads the data
				update_lru(index, tag);
			}
			else if (action == cacheAction::store)
			{
				cache[index][0].data = data;//stores the data
			}
			return true;
		}
		return false;
	}
};
class SACache: public Cache
{
private:
	std::vector<cache_set> setInitAndGetContent()
	{
		cache_unit origin;
		origin.tag = -1;// -1 indicates that the tag value is invalid. We don't use a separate VALID bit. 
		origin.lru_position = 0; // 0 means lowest position
		origin.data = 0;

		const int AMT_OF_WAYS_SA = 4;
		const int AMT_SETS_SA = CACHE_UNITS / AMT_OF_WAYS_SA;

		std::vector<cache_set> cache(AMT_SETS_SA);

		for (int i = 0; i < AMT_SETS_SA; i++)
		{
			//push AMT_OF_WAYS cache units to each cache set
			for (int j = 0; j < AMT_OF_WAYS_SA; j++)
			{
				cache[i].push_back(origin);
			}
		}
		return cache;
	}
public:
	SACache()
		:Cache(setInitAndGetContent())
	{}
	void update_lru(int index, int unit_num)
	{
		//the passed in index and tag is either a hit or when 
			//a new unit is installed
		int prev_lru = cache[index][unit_num].lru_position;
		//set the new index and tag to highest val (3)
		cache[index][unit_num].lru_position = 3;

		//increment the others
		for (int j = 0; j < cache[index].size(); j++)//for all units in the set
		{
			int cur = cache[index][j].lru_position;
			if (cur > prev_lru)
			{
				if (j != unit_num)//if not the current one
				{
					cur--;
					cache[index][j].lru_position = cur;
				}
			}
		}
	}
	void evict(int adr, int data)
	{
		int index = adr & 0x3;
		int tag = (adr & 0xfffffffc) >> 2;

		//first find candidate for eviction
		for (int i = 0; i < cache[index].size(); i++)
		{
			if (cache[index][i].lru_position == 0)
			{
				//update the actual value and tag
				cache[index][i].data = data;
				cache[index][i].tag = tag;

				//update LRU counters
				update_lru(index, i);
				break;
			}
		}
	}
	bool search(int& adr, int& data, cacheAction action) 
	{
		//std::cout << "Searching!\n";
		int index = adr & 0x3;
		int tag = (adr & 0xfffffffc) >> 2;
		std::cout << " At index " << index << "";
		std::cout << " with tag " << tag << "\n";

		for (int j = 0; j < cache[index].size(); j++)
			//for each cache unit in the indexed set
		{
			if (cache[index][j].tag == tag)//a hit!
			{
				if (action == cacheAction::load)
				{
					data = cache[index][j].data;
					update_lru(index, j);
				}
				else if (action == cacheAction::store)
				{
					cache[index][j].data = data;
					//update_lru(index, data);//NOT SURE IF I NEED THIS OR NOT
				}
				return true;
			}
		}
		return false;
	}
};
class FACache: public Cache
{
private:
	std::vector<cache_set> setInitAndGetContent()
	{
		cache_unit origin;
		origin.tag = -1;// -1 indicates that the tag value is invalid. We don't use a separate VALID bit. 
		origin.lru_position = 0; // 0 means lowest position
		origin.data = 0;

		//for FA
		const int AMT_OF_WAYS_FA = 16;
		const int AMT_SETS_FA = 1;
		std::vector<cache_set> cache(AMT_SETS_FA);

		//push AMT_OF_WAYS cache units to each cache set
		for (int j = 0; j < AMT_OF_WAYS_FA; j++)
		{
			cache[0].push_back(origin);
		}
		return cache;
	}
public:
	FACache()
		:Cache(setInitAndGetContent())
	{}
	void update_lru(int index, int tag)
	{
		//the passed in index and tag is either a hit or when 
			//a new unit is installed
		int prev_lru = cache[0][index].lru_position;
		//set the new index and tag to highest val (3)
		cache[0][index].lru_position = 15;

		//increment the others
		for (int j = 0; j < cache[0].size(); j++)//for all units in the set
		{
			int cur = cache[0][j].lru_position;
			if (cur > prev_lru)
			{
				if (j != index)//stops it from reseting the current index
				{
					cur--;
					cache[0][j].lru_position = cur;
				}
			}
		}
	}
	void evict(int adr, int data)
	{
		//int index = adr & 0xf;
		int tag = adr;
		//first find candidate for eviction
		for (int i = 0; i < cache[0].size(); i++)
		{
			if (cache[0][i].lru_position == 0)
			{
				//update the actual value and tag
				cache[0][i].data = data;
				cache[0][i].tag = tag;

				//update LRU counters
				update_lru(i, tag);
				break;
			}
		}
	}
	bool search(int& adr, int& data, cacheAction action) {
		//std::cout << "Searching!\n";
		int tag = adr;
		std::cout << " Tag " << tag << "\n";
		for (int j = 0; j < cache[0].size(); j++)
			//for each cache unit in the set
		{
			if (cache[0][j].tag == tag)//a hit!
			{
				if (action == cacheAction::load)
				{
					data = cache[0][j].data;
					update_lru(j, data);
				}
				else if (action == cacheAction::store)
				{
					cache[0][j].data = data;
					//update_lru(0, data);//NOT SURE IF I NEED THIS OR NOT
				}
				return true;
			}
		}
		return false;
	}
};
#endif
