#include "VMManager.h"

#include <iostream>

VMManager::VMManager(int capacity) : m_capacity(capacity)
{
	//TODO Clean up default values to nullptr

	diskInStream.open("vm.txt");
	diskOutStream.open("vm.txt");

}

int VMManager::store(std::string variableId, unsigned int value) {
	if (m_currentSize < m_capacity) {
		//A frame was found and there is no duplicate variableIds, put the page there
		m_mainMemory[variableId] = &Page(variableId, value);
		m_currentSize++;
		return 1;
	}
	std::cout << "VMManager: store: There is no more space in main memory" << std::endl;
	return -1; //For now if a frame is not found, do nothing and ignore the command
}

int VMManager::release(std::string variableId) {
	std::map<std::string, Page*>::iterator it;
	it = m_mainMemory.find(variableId);

	if (it != m_mainMemory.end()) {
		m_mainMemory.erase(variableId);
		m_currentSize--;
		return 1;
	}
	std::cout << "VMManager: release: Could not find variable " << variableId << " to delete" << std::endl;
	return -1;
}

int VMManager::lookup(std::string variableId) {

	return -1;
}

VMManager::~VMManager()
{
	std::map<std::string, Page*>::iterator it;
	while (it != m_mainMemory.end()) {
		delete it->second;
		it++;
	}
}
