#pragma once
#include "Component.h"
#include "Vector2.h"
#include "json.hpp"
using json = nlohmann::json;
using namespace nlohmann::literals;


namespace FlatEngine
{
	class Transform : public Component
	{
	public:
		Transform();
		~Transform();
		void SetPosition(Vector2 position);
		Vector2 GetPosition();
		std::string GetData();


		//Vector2 position = { 0, 0 };

	private:
		Vector2 position;
		int posX;
		int posY;
		int rotation;
		//Rotation
	};
}
