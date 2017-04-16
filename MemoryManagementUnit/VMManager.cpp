#include "VMManager.h"

#include <iostream>

VMManager::VMManager(int capacity) : m_capacity(capacity)
{
	//TODO Clean up default values to nullptr

	diskInStream.open("vm.txt");
	diskOutStream.open("vm.txt");
}

void VMManager::sweepAges() {
	std::map<std::string, Page*>::iterator it;
	it = m_mainMemory.begin();

	while (it != m_mainMemory.end()) {
		it->second->updateAge();
		it++;
	}
}

int VMManager::store(std::string variableId, unsigned int value) {
	if (m_currentSize < m_capacity) {
		//A frame was found, put the page there
		m_mainMemory[variableId] = new Page(variableId, value);
		m_currentSize++;
		return 1;
	}
	else {
		//There is no more space available. Swapping out page with smallest value of age counter
		std::string smallestAgeId = m_mainMemory.begin()->second->m_variableId;
		int smallestAge = m_mainMemory.begin()->second->m_agingCounter;

		//Finding the variable with the smallest age
		std::map<std::string, Page*>::iterator it;
		it = m_mainMemory.begin();
		while (it != m_mainMemory.end()) {
			if (it->second->m_agingCounter < smallestAge) {
				smallestAgeId = it->second->m_variableId;
				smallestAge = it->second->m_agingCounter;
			}
			it++;
		}
		it = m_mainMemory.find(smallestAgeId);

		std::cout << "SWAP: Variable " << it->second->m_variableId << " with Variable " << variableId << std::endl;

		//Storing the page in disk that is currently in main memory and that will be switched out
		diskStore(it->second->m_variableId, it->second->m_value);
		//Releasing the page from main memory
		release(it->second->m_variableId);
		//Storing the new page into main memory
		store(variableId, value);

		m_mainMemory[variableId]->isUsed();
		return 1;
	}
	std::cout << "VMManager: store: An error occured" << std::endl;
	return -1; //For now if a frame is not found, do nothing and ignore the command
}

int VMManager::diskStore(std::string variableId, unsigned int value) {
	m_diskMemory[variableId] = new Page(variableId, value);
	m_currentDiskSize++;
	return 1;
}

int VMManager::release(std::string variableId) {
	if (m_mainMemory.erase(variableId) > 0) {
		//Found the entry
		m_currentSize--;
		return 1;
	}
	std::cout << "VMManager: release: Could not find variable " << variableId << " to delete" << std::endl;
	return -1;
}

int VMManager::diskRelease(std::string variableId) {
	if (m_diskMemory.erase(variableId) > 0) {
		//Found the entry
		m_currentDiskSize--;
		return 1;
	}
	std::cout << "VMManager: diskRelease: Could not find variable " << variableId << " to delete" << std::endl;
	return -1;
}

int VMManager::lookup(std::string variableId) {
	std::map<std::string, Page*>::iterator it;
	it = m_mainMemory.find(variableId);

	if (it != m_mainMemory.end()) {
		//Found the variable in main memory
		it->second->isUsed();
		return it->second->m_value;
	}

	it = m_diskMemory.find(variableId);

	if (it != m_diskMemory.end()) {
		//Found the entry in disk memory
		int foreValue = it->second->m_value;
		store(variableId, it->second->m_value);
		diskRelease(variableId);
		return foreValue;
	}

	std::cout << "VMManager: lookup: Could not find variable " << variableId << " in main memory or disk" << std::endl;
	return -1;
}

VMManager::~VMManager()
{
	diskInStream.close();
	diskOutStream.close();

	auto it = m_mainMemory.begin();
	while (it != m_mainMemory.end()) {
		delete it->second;
		it++;
	}
}
