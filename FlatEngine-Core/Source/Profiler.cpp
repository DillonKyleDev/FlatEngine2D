#include "FlatEngine.h"
#include "ProfilerProcess.h"
#include <vector>


using Process = FlatEngine::Process;

namespace FlatEngine
{
	std::vector<Process> F_ProfilerProcesses = std::vector<Process>();

	void AddProfilerProcess(std::string name)
	{
		Process process = Process(name);
		F_ProfilerProcesses.push_back(process);
	}

	void AddProcessData(std::string processName, float data)
	{
		for (Process &process : F_ProfilerProcesses)
		{
			if (process.GetProcessName() == processName)
			{
				process.AddHangTimeData(data);
			}
		}
	}

	void RemoveProfilerProcess(std::string toRemove)
	{
		std::vector<Process>::iterator iter = F_ProfilerProcesses.begin();

		while (iter != F_ProfilerProcesses.end())
		{
			if ((*iter).GetProcessName() == toRemove)
			{
				F_ProfilerProcesses.erase(iter);
				return;
			}
			iter++;
		}
	}
}