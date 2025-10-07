#include "Camera.h"
#include "FlatEngine.h"
#include "Transform.h"
#include "GameObject.h"
#include "MappingContext.h"

#include "glm.hpp"


namespace FlatEngine 
{
	Camera::Camera(GameObject* parentPtr, long myID, long parentID)
	{
		SetType(T_Camera);
		SetID(myID);
		SetParentID(parentID);
		m_parentPtr = parentPtr;
		m_b_isPrimaryCamera = false;
		m_b_forceZup = true;
		m_width = 50;
		m_height = 30;
		m_zoom = 10;
		m_lookDirection = Vector3(0.0f, 1.0f, 0.0f);
		m_nearClippingDistance = 0.1f;
		m_farClippingDistance = 100.0f;
		m_perspectiveAngle = 90.0f;
		m_frustrumColor = Vector4(255,255,255,255);
		m_b_shouldFollow = false;
		m_toFollowID = -1;
		m_followSmoothing = 0.1f;
		m_horizontalViewAngle = 0.0f;
		m_verticalViewAngle = 0.0f;
	}

	Camera::~Camera()
	{
	}

	std::string Camera::GetData()
	{
		json jsonData = {
			{ "type", "Camera" },
			{ "id", GetID() },
			{ "_isCollapsed", IsCollapsed() },
			{ "_isActive", IsActive() },
			{ "width", m_width },
			{ "height", m_height },
			{ "_isPrimaryCamera", m_b_isPrimaryCamera },
			{ "zoom", m_zoom },
			{ "frustrumRed", m_frustrumColor.x },
			{ "frustrumGreen", m_frustrumColor.y },
			{ "frustrumBlue", m_frustrumColor.z },
			{ "frustrumAlpha", m_frustrumColor.w },
			{ "_follow", m_b_shouldFollow },
			{ "followSmoothing", m_followSmoothing },
			{ "following", m_toFollowID },
			{ "perspectiveAngle", m_perspectiveAngle },
			{ "nearClippingDistance", m_nearClippingDistance },
			{ "farClippingDistance", m_farClippingDistance },
			{ "horizontalViewAngle", m_horizontalViewAngle },
			{ "verticalViewAngle", m_verticalViewAngle }			
		};

		std::string data = jsonData.dump();
		// Return dumped json object with required data for saving
		return data;
	}

	GameObject* Camera::GetParentPtr()
	{
		return m_parentPtr;
	}

	void Camera::SetPrimaryCamera(bool b_isPrimary)
	{
		m_b_isPrimaryCamera = b_isPrimary;
	}

	bool Camera::IsPrimary()
	{
		return m_b_isPrimaryCamera;
	}

	bool Camera::ForceZUp()
	{
		return m_b_forceZup;
	}

	void Camera::SetForceZUp(bool b_forceZUp)
	{
		m_b_forceZup = b_forceZUp;
	}

	void Camera::SetFrustrumColor(Vector4 color)
	{
		m_frustrumColor = color;
	}

	Vector4 Camera::GetFrustrumColor()
	{
		return m_frustrumColor;
	}

	void Camera::Follow()
	{
		//GameObject *followTarget = GetObjectByID(m_toFollowID);
		//if (m_b_shouldFollow && followTarget != nullptr && followTarget->HasComponent("Transform"))
		//{
		//	Transform* cameraTransform = GetParent()->GetTransform();
		//	Vector2 followPos = followTarget->GetTransform()->GetAbsolutePosition();
		//	Vector2 currentPos = cameraTransform->GetPosition(); // Shouldn't have a parent if following so don't need GetTruePosition()

		//	cameraTransform->SetPosition(Lerp(currentPos, followPos, m_followSmoothing));
		//}
	}

	void Camera::SetShouldFollow(bool b_shouldFollow)
	{
		m_b_shouldFollow = b_shouldFollow;
	}

	bool Camera::GetShouldFollow()
	{
		return m_b_shouldFollow;
	}

	void Camera::SetToFollowID(long toFollow)
	{
		m_toFollowID = toFollow;
	}

	long Camera::GetToFollowID()
	{
		return m_toFollowID;
	}

	void Camera::SetFollowSmoothing(float smoothing)
	{
		m_followSmoothing = smoothing;
	}

	float Camera::GetFollowSmoothing()
	{
		return m_followSmoothing;
	}

	void Camera::SetZoom(float newZoom)
	{
		if (newZoom >= 1 && newZoom <= 100)
		{
			m_zoom = newZoom;
		}
	}

	float Camera::GetZoom()
	{
		return m_zoom;
	}

	glm::vec4 Camera::GetLookDirection()
	{
		float xRotation = 0;
		glm::mat4 rollCameraMatrix;

		if (GetParentPtr() != nullptr && GetParentPtr()->GetName() != "SceneViewCamera")
		{
			Vector3 rotation = m_parentPtr->GetTransform()->GetRotations();
			xRotation = rotation.x;
			m_horizontalViewAngle = rotation.z;
			m_verticalViewAngle = rotation.y;

			rollCameraMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(xRotation), glm::vec3(1.0f, 0.0f, 0.0f));
		}

		glm::mat4 horCameraRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(m_horizontalViewAngle), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 vertCameraRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(m_verticalViewAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 rotationMatrix = horCameraRotationMatrix * vertCameraRotationMatrix;

		if (xRotation != 0)
		{
			rotationMatrix *= rollCameraMatrix;
		}

		glm::vec4 cameraLookDir = rotationMatrix * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

		return cameraLookDir;
	}

	float Camera::GetNearClippingDistance()
	{
		return m_nearClippingDistance;
	}

	void Camera::SetNearClippingDistance(float nearDistance)
	{
		m_nearClippingDistance = nearDistance;
	}

	float Camera::GetFarClippingDistance()
	{
		return m_farClippingDistance;
	}

	void Camera::SetFarClippingDistance(float farDistance)
	{
		m_farClippingDistance = farDistance;
	}

	float Camera::GetPerspectiveAngle()
	{
		return m_perspectiveAngle;
	}

	void Camera::SetPerspectiveAngle(float angle)
	{
		m_perspectiveAngle = angle;
	}

	void Camera::SetHorizontalViewAngle(float angle)
	{
		m_horizontalViewAngle = angle;
	}

	void Camera::SetVerticalViewAngle(float angle)
	{
		m_verticalViewAngle = angle;
	}

	float Camera::GetHorizontalViewAngle()
	{
		return m_horizontalViewAngle;
	}

	float Camera::GetVerticalViewAngle()
	{
		return m_verticalViewAngle;
	}

	void Camera::AddToHorizontalViewAngle(float toAdd)
	{
		m_horizontalViewAngle += toAdd;
	}

	void Camera::AddToVerticalViewAngle(float toAdd)
	{
		if (m_verticalViewAngle + toAdd >= 90)
		{
			m_verticalViewAngle = 89.99f;
		}
		else if (m_verticalViewAngle + toAdd <= -90)
		{
			m_verticalViewAngle = -89.99f;
		}
		else
		{
			m_verticalViewAngle += toAdd;
		}
	}

	void Camera::SetDimensions(float newWidth, float newHeight)
	{
		m_width = newWidth;
		m_height = newHeight;
	}

	float Camera::GetWidth()
	{
		return m_width;
	}

	float Camera::GetHeight()
	{
		return m_height;
	}

	void Camera::AddVelocity(Vector3 velocity)
	{
		m_velocity.x += velocity.x;
		m_velocity.y += velocity.y;
		m_velocity.z += velocity.z;
	}

	Vector3& Camera::GetVelocity()
	{
		return m_velocity;
	}

	// Eventually remove the event handling and give Project viewport camera settings to save and load. Use Primary camera here in the meantime
	void Camera::Update()
	{
		Follow();

		if (m_velocity != 0)
		{
			Transform* transform = GetParent()->GetTransform();
			Vector3 position = transform->GetPosition();
			transform->SetPosition(position + m_velocity);
			m_velocity.x *= 0.95f;
			m_velocity.y *= 0.95f;
			m_velocity.z *= 0.95f;
		}
	}
}