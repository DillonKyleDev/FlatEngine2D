#include "ProfilerProcess.h"


namespace FlatEngine 
{
	Process::Process(std::string name)
	{
		m_processName = name;
		m_rawHangTimeData = std::deque<float>();
		m_tickCounter = 0;
	}

	Process::~Process()
	{
	}

	std::string Process::GetProcessName()
	{
		return m_processName;
	}

	void Process::AddHangTimeData(float hangTime)
	{
		if (m_rawHangTimeData.size() < 100)
		{
			m_rawHangTimeData.push_front(hangTime);
		}
		else if (m_rawHangTimeData.size() >= 100)
		{
			m_rawHangTimeData.pop_front();
			m_rawHangTimeData.push_back(hangTime);
		}

		if (m_tickCounter < 100)
		{
			m_tickCounter++;
		}
		else if (m_tickCounter == 100)
		{
			m_snapShotData.clear();
			for (int i = 0; i < 100; i++)
			{
				m_snapShotData.push_back(m_rawHangTimeData[i]);
			}
			m_tickCounter++;
		}
		else if (m_tickCounter > 100 && m_tickCounter < 200)
		{
			m_tickCounter++;
		}
		else if (m_tickCounter == 200)
		{
			m_tickCounter = 0;
		}

	}

	std::deque<float> Process::GetRawData()
	{
		return m_rawHangTimeData;
	}

	std::vector<float> Process::GetHangTimeData()
	{
		return m_snapShotData;
	}
}