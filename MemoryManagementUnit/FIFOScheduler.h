#pragma once

#include <queue>

#include "MyProcess.h"

class string;

class FIFOScheduler
{
private:
	std::queue<MyProcess> m_processQueue;

	MyProcess* m_currentlyRunningProcessCore1;
	MyProcess* m_currentlyRunningProcessCore2;

public:
	FIFOScheduler(std::string file);
	virtual ~FIFOScheduler();

	void run();
};

