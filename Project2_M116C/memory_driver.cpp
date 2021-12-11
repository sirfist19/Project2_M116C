#include <fstream>
#include <iostream>
#include <cstring>
#include <sstream>
#include <iostream>
#include <vector>
#include "memory_driver.h"
#include "cache.h"
#include "cache_fxns.h"
using namespace std;

/*
Either implement your memory_controller here or use a separate .cpp/.c file for memory_controller and all the other functions inside it (e.g., LW, SW, Search, Evict, etc.)
*/

int main(int argc, char* argv[]) // the program runs like this: ./program <filename> <mode>
{
	// input file (i.e., test.txt)
	string filename = "debug.txt";//argv[1];
	string mode = "2";//argv[2];
	// mode for replacement policy
	int type;

	ifstream fin;

	// opening file
	fin.open(filename.c_str());
	if (!fin) { // making sure the file is correctly opened
		cout << "Error opening " << filename << endl;
		exit(1);
	}

	if (argc > 2)
		type = stoi(argv[2]); // the input could be either 0 or 1 (for DM and FA)
	else type = 0;// the default is DM.


	// reading the text file
	string line;
	vector<trace> myTrace;
	int TraceSize = 0;
	string s1, s2, s3, s4;
	while (getline(fin, line))
	{
		stringstream ss(line);
		getline(ss, s1, ',');
		getline(ss, s2, ',');
		getline(ss, s3, ',');
		getline(ss, s4, ',');
		myTrace.push_back(trace());
		myTrace[TraceSize].MemR = stoi(s1);
		myTrace[TraceSize].MemW = stoi(s2);
		myTrace[TraceSize].adr = stoi(s3);
		myTrace[TraceSize].data = stoi(s4);
		//cout<<myTrace[TraceSize].MemW << endl;
		TraceSize += 1;
	}
	//define memory
	int myMem[MEM_SIZE];
	for (int i = 0; i < MEM_SIZE; i++)
	{
		myMem[i] = 0;
	}

	// initializing
	Cache* cache = initCache(stoi(mode));

	// counters for miss rate
	int accessL = 0; //////
	int accessS = 0;
	int miss = 0; // this has to be updated inside your memory_controller
	int status = 1;
	int clock = 0;
	int traceCounter = 0;
	bool cur_MemR;
	bool cur_MemW;
	int cur_adr;
	int cur_data;
	int hit_count = 0;
	// this is the main loop of the code
	while (traceCounter < TraceSize) {
		if (status == 1)
		{
			cur_MemR = myTrace[traceCounter].MemR;
			cur_MemW = myTrace[traceCounter].MemW;
			cur_data = myTrace[traceCounter].data;
			cur_adr = myTrace[traceCounter].adr;
			//hit = false;
			traceCounter += 1;
			if (cur_MemR == 1)
				accessL += 1;
			else if (cur_MemW == 1)
				accessS += 1;
			cache->print();
			std::cout << "\nClock: " << clock << "\n";
		}
		// YOUR CODE
		//mode is the type of cache to use
		status = memory_controller(cur_MemR, cur_MemW, cur_data, cur_adr, status, 
			miss, mode, cache, myMem, hit_count); 
		// in your memory controller you need to implement your FSM, LW, SW, and MM. 
		////////////
		
		clock += 1;
	}
	// to make sure that the last access is also done
	while (status < 1)
	{
		// YOUR CODE
		status = memory_controller(cur_MemR, cur_MemW, cur_data, 
			cur_adr, status, miss, mode, cache, myMem, hit_count); 
		// in your memory controller you need to implement your FSM, LW, SW, and MM. 
		////////////
		clock += 1;
	}
	cache->print();
	//printMainMem(myMem);
	float miss_rate = miss / (float)accessL;

	// printing the final result
	std::cout << "Miss Count: " << miss;
	std::cout << "\nHit Count: " << hit_count;
	std::cout << "\nTotal Reads: " << accessL;
	std::cout << "\nTotal amount of lw and sw: " << traceCounter;
	std::cout << "\nMiss Rate: " << miss_rate;
	std::cout << "\nTotal Cycles: " << clock;
	// closing the file
	delete cache;
	fin.close();

	return 0;
}
