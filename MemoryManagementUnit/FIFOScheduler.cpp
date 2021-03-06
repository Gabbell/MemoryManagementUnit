#include "FIFOScheduler.h"

#include "MyProcess.h"

#include <string>
#include <fstream>
#include <chrono>
#include <iostream>
#include <sstream>

#include <Windows.h>

void sortProcessArray(MyProcess** array, int arraySize) {
	MyProcess* temp;

	for (int i = 0; i < arraySize; i++) {
		int minIndex = i;

		for (int j = i + 1; j < arraySize; j++) {
			//Sorting by arrival time
			if (array[j]->getArrivalTime() < array[minIndex]->getArrivalTime()) {
				minIndex = j;
			}
		}

		if (minIndex != i) {
			temp = array[i];
			array[i] = array[minIndex];
			array[minIndex] = temp;
		}
	}
}


FIFOScheduler::FIFOScheduler(std::string processFile) : m_currentlyRunningProcessCore1(nullptr), m_currentlyRunningProcessCore2(nullptr), m_runningTime(0) {
	
	std::ifstream input(processFile);

	if (!input) {
		throw std::runtime_error("Error: invalid or missing process.txt file.");
	}

	// Read number of processes
	int numberOfProcesses = 0;
	input >> numberOfProcesses;

	MyProcess** processes = new MyProcess*[numberOfProcesses];

	for (int i = 0; i < numberOfProcesses; i++) {
		std::string pid;
		int arrivalTime;
		int burstTime;
		int priority;

		input >> pid;
		input >> arrivalTime;
		input >> burstTime;
		input >> priority;

		processes[i] = new MyProcess(pid, arrivalTime, burstTime, priority);
	}

	sortProcessArray(processes, numberOfProcesses);

	for (int i = 0; i < numberOfProcesses; i++) {
		m_arrivalQueue.push(processes[i]);
	}

	input.close();

	delete[] processes;

}


FIFOScheduler::~FIFOScheduler() {
	
	while (!m_arrivalQueue.empty()) {
		delete m_arrivalQueue.front();
		m_arrivalQueue.pop();
	}

	while (!m_readyQueue.empty()) {
		delete m_readyQueue.front();
		m_readyQueue.pop();
	}

	if (m_currentlyRunningProcessCore1) {
		delete m_currentlyRunningProcessCore1;
	}

	if (m_currentlyRunningProcessCore2) {
		delete m_currentlyRunningProcessCore2;
	}
}

double getCurrentTime(std::chrono::high_resolution_clock::time_point startTime, std::chrono::high_resolution_clock::time_point endTime);

bool FIFOScheduler::isTerminated() {
	return m_isTerminated;
}

void FIFOScheduler::run() {

	auto startTimePoint = std::chrono::high_resolution_clock::now();

	std::cout << "----------STARTING SCHEDULER----------" << std::endl;

	while (!m_arrivalQueue.empty() || !m_readyQueue.empty() || m_currentlyRunningProcessCore1 || m_currentlyRunningProcessCore2) {
		
		m_runningTime = getCurrentTime(startTimePoint, std::chrono::high_resolution_clock::now());

		// Add process if it has arrived
		while (!m_arrivalQueue.empty() && m_runningTime >= m_arrivalQueue.front()->getArrivalTime()) {
			notifyProcessArrive(m_arrivalQueue.front());
			// Check to see if there is a core availible
			if (!m_currentlyRunningProcessCore1) {
				notifyProcessStart(m_arrivalQueue.front(), 1);
				m_currentlyRunningProcessCore1 = m_arrivalQueue.front();
				startProcess(m_currentlyRunningProcessCore1);
				m_arrivalQueue.pop();
			}
			else if (!m_currentlyRunningProcessCore2) {
				notifyProcessStart(m_arrivalQueue.front(), 2);
				m_currentlyRunningProcessCore2 = m_arrivalQueue.front();
				startProcess(m_currentlyRunningProcessCore2);
				m_arrivalQueue.pop();
			}
			else {
				// No cores availible
				m_readyQueue.push(m_arrivalQueue.front());
				m_arrivalQueue.pop();
			}
		}

		// Start processes in ready queue
		while (!m_readyQueue.empty()) {
			// Check to see if there is a core availible
			if (!m_currentlyRunningProcessCore1) {
				notifyProcessStart(m_readyQueue.front(), 1);
				m_currentlyRunningProcessCore1 = m_readyQueue.front();
				startProcess(m_currentlyRunningProcessCore1);
				m_readyQueue.pop();
			}
			else if (!m_currentlyRunningProcessCore2) {
				notifyProcessStart(m_readyQueue.front(), 2);
				m_currentlyRunningProcessCore2 = m_readyQueue.front();
				startProcess(m_currentlyRunningProcessCore2);
				m_readyQueue.pop();
			}
			else {
				// No cores availible
				break;
			}
		}

		// Check if current running processes are terminated
		if (m_currentlyRunningProcessCore1 && m_currentlyRunningProcessCore1->isTerminated()) {
			WaitForSingleObject(m_currentlyRunningProcessCore1->getHandle(), INFINITE);
			notifyProcessTerminate(m_currentlyRunningProcessCore1, 1);
			delete m_currentlyRunningProcessCore1;

			// Can I replace it?
			if (!m_readyQueue.empty()) {
				notifyProcessStart(m_readyQueue.front(), 1);
				m_currentlyRunningProcessCore1 = m_readyQueue.front();
				startProcess(m_currentlyRunningProcessCore1);
				m_readyQueue.pop();
			}
			else {
				m_currentlyRunningProcessCore1 = nullptr;
			}
		}

		if (m_currentlyRunningProcessCore2 && m_currentlyRunningProcessCore2->isTerminated()) {
			WaitForSingleObject(m_currentlyRunningProcessCore2->getHandle(), INFINITE);
			notifyProcessTerminate(m_currentlyRunningProcessCore2, 2);
			delete m_currentlyRunningProcessCore2;

			// Can I replace it?
			if (!m_readyQueue.empty()) {
				notifyProcessStart(m_readyQueue.front(), 2);
				m_currentlyRunningProcessCore2 = m_readyQueue.front();
				startProcess(m_currentlyRunningProcessCore2);
				m_readyQueue.pop();
			}
			else {
				m_currentlyRunningProcessCore2 = nullptr;
			}
		}
	}

	m_isTerminated = true;
	std::cout << "----------FINISHED----------" << std::endl;
}

DWORD WINAPI dummyRoutine(LPVOID p);

void FIFOScheduler::startProcess(MyProcess* process) {
	HANDLE handle = CreateThread(NULL, 0, dummyRoutine, process, 0, NULL);

	process->setHandle(handle);
}

void writeToOutputLog(std::string msg);

void FIFOScheduler::notifyProcessArrive(MyProcess* process) const {
	std::cout << "Time " << m_runningTime << ", " << process->getPid() << ", Arrived" << std::endl;

	std::stringstream ss;
	ss << "Time " << m_runningTime << ", " << process->getPid() << ", Arrived" << std::endl;
	writeToOutputLog(ss.str());
}

void FIFOScheduler::notifyProcessStart(MyProcess* process, int core) const {
	std::cout << "Time " << m_runningTime << ", " << process->getPid() << ", Started on Core " << core << std::endl;

	std::stringstream ss;
	ss << "Time " << m_runningTime << ", " << process->getPid() << ", Started on Core " << core << std::endl;
	writeToOutputLog(ss.str());
}

void FIFOScheduler::notifyProcessTerminate(MyProcess* process, int core) const {
	std::cout << "Time " << m_runningTime << ", " << process->getPid() << ", Terminated on Core " << core << std::endl;

	std::stringstream ss;
	ss << "Time " << m_runningTime << ", " << process->getPid() << ", Started on Core " << core << std::endl;
	writeToOutputLog(ss.str());
}
