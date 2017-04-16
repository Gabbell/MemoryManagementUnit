#pragma once

#include <queue>

class MyProcess;
class string;

class FIFOScheduler
{
private:
	bool m_isTerminated = false;
	std::queue<MyProcess*> m_arrivalQueue;
	std::queue<MyProcess*> m_readyQueue;

	MyProcess* m_currentlyRunningProcessCore1;
	MyProcess* m_currentlyRunningProcessCore2;

	void startProcess(MyProcess* process);

	void notifyProcessArrive(MyProcess* process, double time, std::ofstream& output) const;
	void notifyProcessStart(MyProcess* process, double time, int core, std::ofstream& output) const;
	void notifyProcessTerminate(MyProcess* process, double time, int core, std::ofstream& output) const;
public:
	FIFOScheduler(std::string file);
	virtual ~FIFOScheduler();

	FIFOScheduler(const FIFOScheduler&) = delete;
	FIFOScheduler& operator=(const FIFOScheduler&) = delete;
	bool isTerminated();
	void run();
};

