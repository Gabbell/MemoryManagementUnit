#pragma once

#include <string>
#include <map>
#include <fstream>
#include "Page.h"
#include "FIFOScheduler.h"
class VMManager
{
private:
	int m_capacity;										//Number of frames in main memory
	int m_currentSize = 0;
	int m_currentDiskSize = 0;
	std::map<std::string, Page*> m_mainMemory;			//Map of pages in main memory
	std::map<std::string, Page*> m_diskMemory;			//Map of memory in disk
	std::ifstream diskInStream;
	std::ofstream diskOutStream;
public:
	VMManager(int capacity);
	~VMManager();

	int store(std::string variableId, unsigned int value);
	int diskStore(std::string variableId, unsigned int value);
	int release(std::string variableId);
	int diskRelease(std::string variableId);
	int lookup(std::string variableId);
	void sweepAges();
	void run();
};

