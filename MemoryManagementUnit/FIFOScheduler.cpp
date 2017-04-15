#include "FIFOScheduler.h"

#include <string>
#include <fstream>

void sortProcessArray(MyProcess* array, int arraySize) {
	MyProcess temp;

	for (int i = 0; i < arraySize; i++) {
		int minIndex = i;

		for (int j = i + 1; j < arraySize; j++) {
			//Sorting by arrival time
			if (array[j].getArrivalTime() < array[minIndex].getArrivalTime()) {
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


FIFOScheduler::FIFOScheduler(std::string processFile) {
	
	std::ifstream input(processFile);

	if (!input) {
		throw std::runtime_error("Error: invalid process.txt file.");
	}

	// Read number of processes
	int numberOfProcesses = 0;
	input >> numberOfProcesses;

	MyProcess* processes = new MyProcess[numberOfProcesses];

	for (int i = 0; i < numberOfProcesses; i++) {
		std::string pid;
		int arrivalTime;
		int burstTime;
		int priority;

		input >> pid;
		input >> arrivalTime;
		input >> burstTime;
		input >> priority;

		processes[i] = MyProcess(pid, arrivalTime, burstTime, priority);
	}

	sortProcessArray(processes, numberOfProcesses);


}


FIFOScheduler::~FIFOScheduler()
{
}

void FIFOScheduler::run() {

}
