#pragma once
#include <string>
#include <imgui.h>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"


namespace FlatEngine
{
	class Logger
	{
	public:
		Logger();
		~Logger();

		void LogString(std::string line, std::string from);
		void LogFloat(float variable, std::string line = "", std::string from = "[C++]");
		void LogDouble(double variable, std::string line = "", std::string from = "[C++]");
		void LogLong(long variable, std::string line = "", std::string from = "[C++]");
		void LogInt(int variable, std::string line = "", std::string from = "[C++]");
		void LogVector2(Vector2 vector, std::string line = "", std::string from = "[C++]");
		void LogVector3(Vector3 vector, std::string line = "", std::string from = "[C++]");
		void LogVector4(Vector4 vector, std::string line = "", std::string from = "[C++]");
		void LogSeparator();
		void DrawRectangle(Vector2 startingPoint, Vector2 endingPoint, Vector4 color, float thickness, ImDrawList* drawList);
		void DrawLine(Vector2 startingPoint, Vector2 endingPoint, Vector4 color, float thickness, ImDrawList* drawList);
		void DrawPoint(Vector2 point, Vector4 color, ImDrawList* drawlist);
		ImGuiTextBuffer &GetBuffer();
		void ClearBuffer();

	private:
		ImGuiTextBuffer log;
	};
}

