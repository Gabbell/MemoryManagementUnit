#include <iostream>
#include <chrono>
#include <string>
#include <Windows.h>

#include "FIFOScheduler.h"
#include "MyProcess.h"

#include <iostream>

using namespace std;

double getCurrentTime(std::chrono::high_resolution_clock::time_point startTime, std::chrono::high_resolution_clock::time_point endTime) {

	using namespace std::chrono;

	//Calculating time span between constructor starTime and now
	duration<double> time_span = duration_cast<duration<double>>(endTime - startTime);

	//Return in ms
	return time_span.count() * 1000;
}

DWORD WINAPI dummyRoutine(LPVOID p) {
	std::chrono::high_resolution_clock::time_point t_start = std::chrono::high_resolution_clock::now();
	MyProcess* process = (MyProcess*)(p);

	//Busy waiting
	while (getCurrentTime(t_start, std::chrono::high_resolution_clock::now()) < process->getBurstTime());

	// Terminate process
	process->terminate();

	return 0;
}

DWORD WINAPI overwatchRoutine(LPVOID p) {
	//Scheduler scheduler;
	
	//scheduler.run(&dummyRoutine);
	try {
		FIFOScheduler scheduler("processes.txt");
		scheduler.run();
	}
	catch (runtime_error& e) {
		cout << e.what() << endl;
	}

	return 0;
}

int main() {

	//Create Overwatch thread
	DWORD(WINAPI *ow_routine)(LPVOID) = &overwatchRoutine;

	HANDLE t_overwatch = CreateThread(
		NULL,										//Default security attributes
		0,											//Default executable stack size
		(LPTHREAD_START_ROUTINE)ow_routine,			//Pointer to function
		NULL,										//Don't need it
		0,											//Will start as soon as created
		NULL);										//Do not need the thread ID

	WaitForSingleObject(t_overwatch, INFINITE);

	system("pause"); // Used for testing

	return 0;
}