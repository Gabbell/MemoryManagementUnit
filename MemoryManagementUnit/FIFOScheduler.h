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


	double m_runningTime;

	void startProcess(MyProcess* process);

	void notifyProcessArrive(MyProcess* process) const;
	void notifyProcessStart(MyProcess* process, int core) const;
	void notifyProcessTerminate(MyProcess* process, int core) const;
public:
	FIFOScheduler(std::string file);
	virtual ~FIFOScheduler();
	bool isTerminated();
	void run();

	double getRunningTime() const { return m_runningTime; };

	FIFOScheduler(const FIFOScheduler&) = delete;
	FIFOScheduler& operator=(const FIFOScheduler&) = delete;
};

