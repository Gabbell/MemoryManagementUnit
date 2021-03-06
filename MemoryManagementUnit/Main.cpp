#include "FIFOScheduler.h"
#include "MyProcess.h"
#include "VMManager.h"
#include <sstream>

using namespace std;

VMManager* mmu = nullptr;
FIFOScheduler* scheduler = nullptr;
HANDLE lock;

struct Command {
	enum Type {
		STORE,
		LOOKUP,
		RELEASE
	};

	Type type;
	std::string variableId;
	unsigned int value;
};

vector<Command> commands;

ofstream outputLog;
HANDLE logLock;

void writeToOutputLog(std::string msg) {
	WaitForSingleObject(logLock, INFINITE);
	outputLog << msg;
	ReleaseMutex(logLock);
}

void synchronizedStore(std::string variableId, unsigned int value, MyProcess* process) {
	WaitForSingleObject(lock, INFINITE);
	
	mmu->store(variableId, value, process);
	
	ReleaseMutex(lock);
}

void synchronizedRelease(std::string variableId) {
	WaitForSingleObject(lock, INFINITE);

	mmu->release(variableId);

	ReleaseMutex(lock);
}

int synchronizedLookup(std::string variableId, MyProcess* process) {
	WaitForSingleObject(lock, INFINITE);
	
	int value = mmu->lookup(variableId, process);

	ReleaseMutex(lock);

	return value;
}
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

	std::chrono::high_resolution_clock::time_point commandTimer = std::chrono::high_resolution_clock::now();

	int commandIndex = 0;
	int commandWait = rand() % 201;

	//Busy waiting
	while (getCurrentTime(t_start, std::chrono::high_resolution_clock::now()) < process->getBurstTime()) {
		if ((getCurrentTime(commandTimer, std::chrono::high_resolution_clock::now()) >= commandWait) && commandIndex < commands.size()) {
			Command cmd = commands[commandIndex];

			switch (cmd.type) {
			case Command::STORE: {
				cout << "Time " << scheduler->getRunningTime() << ", " << process->getPid() << ", Store: Variable " << cmd.variableId << ", Value " << cmd.value << std::endl;

				stringstream ss;
				ss << "Time " << scheduler->getRunningTime() << ", " << process->getPid() << ", Store: Variable " << cmd.variableId << ", Value " << cmd.value << std::endl;
				writeToOutputLog(ss.str());

				synchronizedStore(cmd.variableId, cmd.value, process);
				break;
			}
			case Command::RELEASE: {
				cout << "Time " << scheduler->getRunningTime() << ", " << process->getPid() << ", Release: Variable " << cmd.variableId << std::endl;

				stringstream ss;
				ss << "Time " << scheduler->getRunningTime() << ", " << process->getPid() << ", Release: Variable " << cmd.variableId << std::endl;
				writeToOutputLog(ss.str());

				synchronizedRelease(cmd.variableId);
				break;
			}
			case Command::LOOKUP: {
				int value = synchronizedLookup(cmd.variableId, process);
				cout << "Time " << scheduler->getRunningTime() << ", " << process->getPid() << ", Lookup: Variable " << cmd.variableId << ", Value " << value << std::endl;

				stringstream ss;
				ss << "Time " << scheduler->getRunningTime() << ", " << process->getPid() << ", Lookup: Variable " << cmd.variableId << ", Value " << value << std::endl;
				writeToOutputLog(ss.str());

				break;
			}
			default:
				break;
			}
			commandTimer = std::chrono::high_resolution_clock::now();
			commandIndex++;
			commandWait = rand() % 201;
		}
	}

	// Terminate process
	process->terminate();

	return 0;
}

DWORD WINAPI overwatchRoutine(LPVOID p) {
	std::chrono::high_resolution_clock::time_point t_start = std::chrono::high_resolution_clock::now();
	int agingWait = 100;

	while (!scheduler->isTerminated()) {
		double now = getCurrentTime(t_start, std::chrono::high_resolution_clock::now());
		if (now >= agingWait) {
			//Aging tick
			WaitForSingleObject(lock, INFINITE);
			mmu->sweepAges();
			ReleaseMutex(lock);

			agingWait += 100;
		}
	}

	return 0;
}

int main() {

	outputLog.open("output.txt");
	logLock = CreateMutex(NULL, false, NULL);

	// Load commands
	ifstream commandFile("commands.txt");

	if (!commandFile) {
		cout << "Error: invalid or missing commands.txt file." << endl;
		return 1;
	}

	while (!commandFile.eof()) {
		std::string type;
		std::string variableId;
		
		commandFile >> type;
		commandFile >> variableId;

		if (type == "Store") {
			unsigned int value = 0;
			commandFile >> value;
			commands.push_back({Command::STORE, variableId, value});
		}
		else if (type == "Release") {
			commands.push_back({ Command::RELEASE, variableId, 0 });
		}
		else if (type == "Lookup") {
			commands.push_back({ Command::LOOKUP, variableId, 0 });
		}
	}

	commandFile.close();

	// Create MMU
	ifstream configFile("memconfig.txt");

	if (!configFile) {
		cout << "Error: invalid or missing memconfig.txt file." << endl;
		return 1;
	}

	int capacity = 0;
	configFile >> capacity;

	configFile.close();

	mmu = new VMManager(capacity);

	// Create mutex
	lock = CreateMutex(NULL, false, NULL);

	try {
		scheduler = new FIFOScheduler("processes.txt");
	}
	catch (runtime_error& e) {
		cout << e.what() << endl;
	}

	// Create Overwatch thread
	DWORD(WINAPI *ow_routine)(LPVOID) = &overwatchRoutine;

	HANDLE t_overwatch = CreateThread(
		NULL,										//Default security attributes
		0,											//Default executable stack size
		(LPTHREAD_START_ROUTINE)ow_routine,			//Pointer to function
		NULL,										//Don't need it
		CREATE_SUSPENDED,							//Will start as soon as created
		NULL);										//Do not need the thread ID
	ResumeThread(t_overwatch);

	scheduler->run();
	
	WaitForSingleObject(t_overwatch, INFINITE);
	CloseHandle(t_overwatch);
	CloseHandle(lock);

	// Destroy MMU
	delete scheduler;
	delete mmu;

	outputLog.close();
	CloseHandle(logLock);

	system("pause"); // Used for testing

	return 0;
}