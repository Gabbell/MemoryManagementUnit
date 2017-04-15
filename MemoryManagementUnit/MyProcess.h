#pragma once

#include <string>
#include <Windows.h>
#include <chrono>
#include <iostream>

typedef std::chrono::high_resolution_clock HRClock;

class MyProcess
{
private:
	std::string m_pid;					// Unique identifier for the process	
	int m_priority;						// Priority of the process
	double m_arrival_time;				// Arrival time of the process
	double m_burst_time;				// Execution time of the process
	HANDLE m_handle;					// Handle to thread provided by Windows

	bool m_terminated;					// True if the process has terminated
public:
	MyProcess(std::string pid, double arrival_time, double burst_time, int priority);
	~MyProcess();

	void terminate() {
		m_terminated = true;
	}

	// Getters
	std::string getPid() const {
		return m_pid;
	}
	
	double getArrivalTime() const {
		return m_arrival_time;
	}
	
	double getBurstTime() const {
		return m_burst_time;
	}
	
	int getPriority() const {
		return m_priority;
	}

	HANDLE getHandle() const {
		return m_handle;
	}
	
	bool isTerminated() const {
		return m_terminated;
	}

	// Setters
	void setPriority(int priority) {
		m_priority = priority;
	}

	void setHandle(HANDLE t_handle) {
		m_handle = t_handle;
	}

};

