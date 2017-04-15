#pragma once

#include <string>
#include <map>
#include <fstream>
#include "Page.h"
class VMManager
{
private:
	int m_capacity;										//Number of frames in main memory
	int m_currentSize = 0;
	std::map<std::string, Page*> m_mainMemory;			//Pointer to array of pages in main memory
	std::map<std::string, Page*> m_diskMemory;			//Vector of memory in disk
	std::ifstream diskInStream;
	std::ofstream diskOutStream;
public:
	VMManager(int capacity);
	~VMManager();

	int store(std::string variableId, unsigned int value);
	int release(std::string variableId);
	int lookup(std::string variableId);
};

