#include "BoxCollider.h"
#include "FlatEngine.h"
#include "GameObject.h"

namespace FlatEngine
{
	BoxCollider::BoxCollider(long myID, long parentID)
	{
		SetType(ComponentTypes::BoxCollider);
		SetID(myID);
		SetParentID(parentID);
		activeWidth = 5;
		activeHeight = 3;
		activeOffset = Vector2(0, 0);
		activeLayer = 0;

		// Initialize callback functions to nullptr
		OnActiveCollision = nullptr;
		OnCollisionEnter = nullptr;
		OnCollisionLeave = nullptr;
	}

	BoxCollider::BoxCollider(std::shared_ptr<BoxCollider> toCopy, long newParentID)
	{
		SetType(ComponentTypes::BoxCollider);
		SetID(GetNextComponentID());
		SetParentID(newParentID);
		SetActive(toCopy->IsActive());
		activeWidth = toCopy->GetActiveWidth();
		activeHeight = toCopy->GetActiveHeight();
		activeOffset = toCopy->GetActiveOffset();
		activeLayer = toCopy->GetActiveLayer();

		// Initialize callback functions to nullptr
		OnActiveCollision = toCopy->OnActiveCollision;
		OnCollisionEnter = toCopy->OnCollisionEnter;
		OnCollisionLeave = toCopy->OnCollisionLeave;
	}

	BoxCollider::~BoxCollider()
	{
	}

	void BoxCollider::SetOnColliding(std::function<void(std::shared_ptr<GameObject>, std::shared_ptr<GameObject>)> callback)
	{
		OnActiveCollision = callback;
		_onActiveCollidingSet = true;
	}

	void BoxCollider::SetOnCollisionEnter(std::function<void(std::shared_ptr<GameObject>, std::shared_ptr<GameObject>)> callback)
	{
		OnCollisionEnter = callback;
		_onCollisionEnterSet = true;
	}

	void BoxCollider::SetOnCollisionLeave(std::function<void(std::shared_ptr<GameObject>, std::shared_ptr<GameObject>)> callback)
	{
		OnCollisionLeave = callback;
		_onCollisionLeaveSet = true;
	}

	bool BoxCollider::IsColliding()
	{
		return _isColliding;
	}

	void BoxCollider::SetColliding(bool _colliding)
	{
		_isColliding = _colliding;
	}

	void BoxCollider::RemoveCollidingObject(std::shared_ptr<GameObject> object)
	{
		for (std::vector<std::shared_ptr<GameObject>>::iterator iterator = collidingObjects.begin(); iterator != collidingObjects.end();)
		{
			if ((*iterator)->GetID() == object->GetID())
				collidingObjects.erase(iterator);

			iterator++;
		}
	}

	void BoxCollider::AddCollidingObject(std::shared_ptr<GameObject> object)
	{
		collidingObjects.push_back(object);
	}

	std::vector<std::shared_ptr<GameObject>> BoxCollider::GetCollidingObjects()
	{
		return collidingObjects;
	}

	void BoxCollider::SetActiveDimensions(float width, float height)
	{
		if (width >= 0 && height >= 0)
		{
			activeWidth = width;
			activeHeight = height;
		}
		else
			FlatEngine::LogString("The active width or height you tried to set to BoxCollider component was < 0. Try again.");
	}

	void BoxCollider::SetActiveOffset(Vector2 offset)
	{
		activeOffset = offset;
	}

	void BoxCollider::SetActiveLayer(int layer)
	{
		if (layer >= 0)
			activeLayer = layer;
		else
			FlatEngine::LogString("BoxCollider active layer must be an integer greater than 0.");
	}

	int BoxCollider::GetActiveLayer()
	{
		return activeLayer;
	}

	float BoxCollider::GetActiveWidth()
	{
		return activeWidth;
	}

	float BoxCollider::GetActiveHeight()
	{
		return activeHeight;
	}

	Vector2 BoxCollider::GetActiveOffset()
	{
		return activeOffset;
	}

	void BoxCollider::SetActiveEdges(ImVec4 edges)
	{
		activeEdges = edges;
	}

	ImVec4 BoxCollider::GetActiveEdges()
	{
		return activeEdges;
	}

	std::string BoxCollider::GetData()
	{
		json jsonData = {
			{ "type", "BoxCollider" },
			{ "id", GetID() },
			{ "_isCollapsed", IsCollapsed() },
			{ "_isActive", IsActive() },
			{ "activeWidth", activeWidth },
			{ "activeHeight", activeHeight },
			{ "activeOffsetX", activeOffset.x },
			{ "activeOffsetY", activeOffset.y },
			{ "activeLayer", activeLayer },
		};

		std::string data = jsonData.dump();
		// Return dumped json object with required data for saving
		return data;
	}

	void BoxCollider::SetConnectedScript(std::string scriptName)
	{
		connectedScript = scriptName;
	}

	std::string BoxCollider::GetConnectedScript()
	{
		return connectedScript;
	}
}
