#include "VMManager.h"

#include <fstream>
#include <iostream>

VMManager::VMManager(int capacity) : m_capacity(capacity)
{
	//TODO Clean up default values to nullptr

	diskInStream.open("vm.txt");
	diskOutStream.open("vm.txt");


}

void VMManager::store(std::string variableId, unsigned int value) {
	for (int i = 0; i < m_capacity; i++) {
		if (m_currentSize < m_capacity && !checkDuplicate(variableId)) {
			//A frame was found and there is no duplicate variableIds, put the page there
			m_mainMemory[variableId] = Page(variableId, value);
			m_currentSize++;
		}
	}
	std::cout << "VMManager: store: There is no more space in main memory" << std::endl;
	return; //For now if a slot is not found, do nothing and ignore the command
}

void VMManager::Release(std::string variableId) {
}

VMManager::~VMManager()
{
}
