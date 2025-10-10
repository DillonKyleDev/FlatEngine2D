#pragma once
#include <string>
#include <deque>
#include <vector>


namespace FlatEngine
{
	class Process
	{
	public:
		Process(std::string name);
		~Process();

		std::string GetProcessName();
		void AddHangTimeData(float hangTime);
		std::deque<float> GetRawData();
		std::vector<float> GetHangTimeData();

	private:
		std::string m_processName;
		std::deque<float> m_rawHangTimeData;
		std::vector<float> m_snapShotData;
		int m_tickCounter;
	};
}