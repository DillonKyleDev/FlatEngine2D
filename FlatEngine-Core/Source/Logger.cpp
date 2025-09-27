#include "Logger.h"


namespace FlatEngine
{
	Logger::Logger()
	{
		log = ImGuiTextBuffer();
	}

	Logger::~Logger()
	{
	}

	void Logger::LogString(std::string line, std::string from)
	{
		line = from + " " + line + "\n";
		log.appendf(line.c_str());
	}

	void Logger::LogFloat(float variable, std::string line, std::string from)
	{
		line = from + " " + line + std::to_string(variable) + "\n";
		log.appendf(line.c_str());
	}

	void Logger::LogDouble(double variable, std::string line, std::string from)
	{
		line = from + " " + line + std::to_string(variable) + "\n";
		log.appendf(line.c_str());
	}

	void Logger::LogLong(long variable, std::string line, std::string from)
	{
		line = from + " " + line + std::to_string(variable) + "\n";
		log.appendf(line.c_str());
	}

	void Logger::LogInt(int variable, std::string line, std::string from)
	{
		line = from + " " + line + std::to_string(variable) + "\n";
		log.appendf(line.c_str());
	}

	void Logger::LogVector2(Vector2 vector, std::string line, std::string from)
	{
		line = from + " " + line + " x: " + std::to_string(vector.x) + " y: " + std::to_string(vector.y) + "\n";
		log.appendf(line.c_str());
	}

	void Logger::LogVector3(Vector3 vector, std::string line, std::string from)
	{
		line = from + " " + line + " x: " + std::to_string(vector.x) + " y: " + std::to_string(vector.y) + " z: " + std::to_string(vector.z) + "\n";
		log.appendf(line.c_str());
	}

	void Logger::LogVector4(Vector4 vector, std::string line, std::string from)
	{
		line = from + " " + line + " x: " + std::to_string(vector.x) + " y: " + std::to_string(vector.y) + " z: " + std::to_string(vector.z) + " w: " + std::to_string(vector.w) + "\n";
		log.appendf(line.c_str());
	}

	void Logger::LogSeparator()
	{
		std::string line = "----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n";
		log.appendf(line.c_str());
	}

	void Logger::DrawRectangle(Vector2 startingPoint, Vector2 endingPoint, Vector4 color, float thickness, ImDrawList* drawList)
	{
		drawList->AddRect(Vector2(startingPoint.x, startingPoint.y), Vector2(endingPoint.x, endingPoint.y), ImGui::GetColorU32(color), 0.0f, 0, thickness);
	}

	void Logger::DrawLine(Vector2 startingPoint, Vector2 endingPoint, Vector4 color, float thickness, ImDrawList* drawList)
	{
		drawList->AddLine(Vector2(startingPoint.x, startingPoint.y), Vector2(endingPoint.x, endingPoint.y), ImGui::GetColorU32(color), thickness);
	}

	void Logger::DrawPoint(Vector2 point, Vector4 color, ImDrawList* drawList)
	{
		drawList->AddLine(Vector2(point.x, point.y), Vector2(point.x + 1, point.y), ImGui::GetColorU32(color));
		drawList->AddLine(Vector2(point.x, point.y), Vector2(point.x, point.y - 1), ImGui::GetColorU32(color));
		drawList->AddLine(Vector2(point.x + 1, point.y + 1), Vector2(point.x + 1, point.y + 1), ImGui::GetColorU32(color));
	}

	ImGuiTextBuffer &Logger::GetBuffer()
	{
		return log;
	}

	void Logger::ClearBuffer()
	{
		log.clear();
	}
}