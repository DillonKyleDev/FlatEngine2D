#pragma once
#include "Component.h"
#include "Vector3.h"
#include "Vector4.h"

#include <string>


namespace FlatEngine
{
	enum LightType {
		LT_None,
		LT_Directional,		
		LT_Point
	};
	class Light : public Component
	{
	public:
		Light(long myID = -1, long parentID = -1);
		~Light();
		std::string GetData();

		void SetLightType(LightType lightType);
		LightType GetLightType();
		void SetDirection(Vector3 direction);
		Vector3 GetDirection();
		void SetColor(Vector4 color);
		Vector4 GetColor();

	private:
		LightType m_lightType;
		Vector3 m_direction;
		Vector4 m_color;
	};
}
