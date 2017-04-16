#include "VMManager.h"

#include <iostream>
#include <sstream>

VMManager::VMManager(int capacity) : m_capacity(capacity)
{
	//TODO Clean up default values to nullptr

	diskInStream.open("vm.txt");
	diskOutStream.open("vm.txt", std::ofstream::app|std::ofstream::out);
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
	diskOutStream << variableId << " " << value << std::endl;
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
	std::string currentId;
	std::string currentValue;
	std::stringstream strim;

	while (!diskInStream.eof()) {
		diskInStream >> currentId;
		diskInStream >> currentValue;

		if (currentId != variableId) {
			strim << currentId << " " << currentValue << std::endl;
		}
		else {
			//Found the variableId
			m_currentDiskSize--;
		}
	}
	diskOutStream << strim.str();
	return 1;
}

int VMManager::lookup(std::string variableId) {
	std::map<std::string, Page*>::iterator it;
	it = m_mainMemory.find(variableId);

	if (it != m_mainMemory.end()) {
		//Found the variable in main memory
		it->second->isUsed();
		return it->second->m_value;
	}

	std::string currentId;
	int currentValue;
	std::stringstream strim;

	while (!diskInStream.eof()) {
		diskInStream >> currentId;
		diskInStream >> currentValue;

		if (currentId == variableId) {
			//Found the variableId
			store(variableId, currentValue);
			diskRelease(variableId);
			return currentValue;
		}
	}
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
