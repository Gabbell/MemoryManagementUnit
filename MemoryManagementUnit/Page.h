#pragma once

#include <string>

typedef unsigned char uint8;

struct Page {
	std::string m_variableId;
	unsigned int m_value;
	uint8 m_agingCounter = 0b00000000;
	bool m_wasUsed = false;

	Page(std::string variableId, unsigned int value) {
		m_variableId = variableId;
		m_value = value;
	}
	void updateAge() {
		m_agingCounter = m_agingCounter >> 1;
		if (m_wasUsed) {
			m_agingCounter = m_agingCounter | 0b10000000;
		}	
		m_wasUsed = false;
	}
	void isUsed() {
		m_wasUsed = true;
	}
};
