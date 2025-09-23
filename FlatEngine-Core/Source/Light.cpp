#include "Light.h"

#include "json.hpp"
using json = nlohmann::json;


namespace FlatEngine
{
	Light::Light(long myID, long parentID)
	{
		SetType(T_Light);
		SetID(myID);
		SetParentID(parentID);

		m_lightType = LightType::LT_Point;
		m_direction = Vector3(0, 0, 1);
		m_color = Vector4(1);
	}

	Light::~Light()
	{
	}

	std::string Light::GetData()
	{
		json jsonData = {
			{ "type", "Light" },
			{ "id", GetID() },
			{ "_isCollapsed", IsCollapsed() },
			{ "_isActive", IsActive() },
			{ "directionX", m_direction.x },
			{ "directionY", m_direction.y },
			{ "directionZ", m_direction.z },
			{ "colorX", m_color.x },
			{ "colorY", m_color.y },
			{ "colorZ", m_color.z },
			{ "colorW", m_color.w },
			{ "lightType", (int)m_lightType }
		};

		std::string data = jsonData.dump();
		// Return dumped json object with required data for saving
		return data;
	}


	void Light::SetLightType(LightType lightType)
	{
		m_lightType = lightType;
	}

	LightType Light::GetLightType()
	{
		return m_lightType;
	}

	void Light::SetDirection(Vector3 direction)
	{
		m_direction = direction;
	}

	Vector3 Light::GetDirection()
	{
		return m_direction;
	}

	void Light::SetColor(Vector4 color)
	{
		m_color = color;
	}

	Vector4 Light::GetColor()
	{
		return m_color;
	}
}
