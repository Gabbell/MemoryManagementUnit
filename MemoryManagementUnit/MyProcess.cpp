#include "MyProcess.h"

MyProcess::MyProcess(std::string pid, double arrival_time, double burst_time, int priority):
	m_pid(pid), 
	m_arrival_time(arrival_time), 
	m_burst_time(burst_time), 
	m_priority(priority), 
	m_terminated(false)
{}

MyProcess::~MyProcess() {
	CloseHandle(m_handle);
}
