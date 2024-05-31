#include "RigidBody.h"
#include "Transform.h"
#include "CharacterController.h"
#include "BoxCollider.h"


namespace FlatEngine {
	RigidBody::RigidBody(long myID, long parentID)
	{
		SetType(ComponentTypes::RigidBody);
		SetID(myID);
		SetParentID(parentID);
		mass = 1;
		angularDrag = 1;
		gravity = 1;
		fallingGravity = gravity * 1.2f;
		gravityCorrection = 1;
		velocity = Vector2(0, 0);
		pendingForces = Vector2(0, 0);
		instantForces = Vector2(0, 0);
		acceleration = Vector2(0, 0);
		terminalVelocity = gravity * 0.4f;
		windResistance = 1.0f;  // Lower value = more resistance
		friction = 0.86f;  // 1 = no friction. 0 = velocity = 0
		equilibriumForce = 2;
		forceCorrection = 0.03f;
		_isMoving = false;
		_isContinious = false;
		_isGrounded = false;
		_isKinematic = false;
		_isStatic = false;
	}

	RigidBody::RigidBody(std::shared_ptr<RigidBody> toCopy, long newParentID)
	{
		SetType(ComponentTypes::RigidBody);
		SetID(GetNextComponentID());
		SetParentID(newParentID);
		mass = toCopy->GetMass();
		angularDrag = toCopy->GetAngularDrag();
		gravity = toCopy->GetGravity();
		velocity = toCopy->GetVelocity();
		pendingForces = Vector2(0, 0);
		instantForces = Vector2(0, 0);
		acceleration = Vector2(0, 0);
		terminalVelocity = toCopy->GetTerminalVelocity();
		windResistance = toCopy->windResistance;
		friction = toCopy->friction;
		equilibriumForce = toCopy->GetEquilibriumForce();
		forceCorrection = forceCorrection;
		_isMoving = toCopy->IsMoving();
		_isContinious = toCopy->IsContinuous();
		_isGrounded = toCopy->IsGrounded();
		_isKinematic = toCopy->IsKinematic();
		_isStatic = toCopy->IsStatic();
	}

	RigidBody::~RigidBody()
	{
	}

	std::string RigidBody::GetData()
	{
		json jsonData = {
			{ "type", "RigidBody" },
			{ "id", GetID() },
			{ "_isCollapsed", IsCollapsed() },
			{ "_isActive", IsActive() },
			{ "mass", mass},
			{ "angularDrag", angularDrag },
			{ "gravity", gravity },
			{ "terminalVelocity", terminalVelocity },
			{ "windResistance", windResistance },
			{ "_isContinious", _isContinious },
			{ "_isKinematic", _isKinematic },
			{ "_isStatic", _isStatic },
		};

		std::string data = jsonData.dump();
		// Return dumped json object with required data for saving
		return data;
	}

	void RigidBody::CalculatePhysics()
	{
		// Add up forces
		ApplyGravity();
		ApplyFriction();
		ApplyEquilibriumForce();
		ApplyCollisionForces();

		// Apply them to RigidBody
		if (loadedProject->GetPhysicsSystem() == "Euler")
		{
			CalculateEulerPhysics();
		}
		else if (loadedProject->GetPhysicsSystem() == "Verlet")
		{
			// CalculateVerletPhysics();
		}
	}

	void RigidBody::CalculateEulerPhysics()
	{
		acceleration = Vector2(pendingForces.x / mass, pendingForces.y / mass);
	}

	void RigidBody::CalculateVerletPhysics()
	{
		// TODO
	}

	void RigidBody::ApplyPhysics(float deltaTime)
	{
		if (loadedProject->GetPhysicsSystem() == "Euler")
		{
			ApplyEulerPhysics(deltaTime);			
		}
		else if (loadedProject->GetPhysicsSystem() == "Verlet")
		{
			// ApplyVerletPhysics(deltaTime);
		}
	}

	void RigidBody::ApplyEulerPhysics(float deltaTime)
	{
		// Then apply to transform
		velocity = Vector2(acceleration.x * deltaTime, acceleration.y * deltaTime);
		std::shared_ptr<FlatEngine::Transform> transform = GetParent()->GetTransformComponent();
		Vector2 position = transform->GetTruePosition();
		LogFloat(velocity.y, "Velocity.y: ");

		transform->SetPosition(Vector2(position.x + velocity.x, position.y + velocity.y));
	}

	void RigidBody::ApplyVerletPhysics(float deltaTime)
	{
		// TODO
	}

	Vector2 RigidBody::AddVelocity(Vector2 vel)
	{
		// Make sure not colliding in that direction before adding the velocity
		std::shared_ptr<FlatEngine::BoxCollider> boxCollider = GetParent()->GetBoxCollider();
		if (vel.x > 0 && (!boxCollider->_isCollidingRight || !boxCollider->_rightCollisionStatic) || vel.x < 0 && (!boxCollider->_isCollidingLeft || !boxCollider->_leftCollisionStatic))
			pendingForces.x += vel.x;

		pendingForces.y += vel.y;
		return pendingForces;
	}

	void RigidBody::ApplyGravity()
	{
		if (gravity > 0)
		{
			if (!_isGrounded && velocity.y > -terminalVelocity)
			{
				if (velocity.y < 0)
					pendingForces.y -= fallingGravity;
				else
					pendingForces.y -= gravity;
			}
		}
		else if (gravity < 0)
		{
			if (!_isGrounded && velocity.y < terminalVelocity)
			{
				if (velocity.y > 0)
					pendingForces.y -= fallingGravity;
				else
					pendingForces.y -= gravity;
			}
		}
	}

	void RigidBody::ApplyFriction()
	{
		// Wind Friction
		if (!_isGrounded)
		{
			Vector2 dampenedVelocity = Vector2(pendingForces.x * windResistance, pendingForces.y * windResistance);
			pendingForces = dampenedVelocity;
		}

		// Get Character Controller for _isMoving
		std::shared_ptr<FlatEngine::CharacterController> characterController = nullptr;

		if (GetParent() != nullptr && GetParent()->HasComponent("CharacterController"))
			characterController = GetParent()->GetCharacterController();

		bool _isMoving = false;
		if (characterController != nullptr)
			_isMoving = characterController->IsMoving();

		// Ground Friction
		if (!_isMoving && _isGrounded)
		{
			Vector2 dampenedVelocity = Vector2(pendingForces.x * friction, pendingForces.y);
			pendingForces = dampenedVelocity;
		}
	}

	void RigidBody::ApplyEquilibriumForce()
	{
		float maxSpeed = 1;
		std::shared_ptr<FlatEngine::CharacterController> characterController = nullptr;

		if (GetParent() != nullptr && GetParent()->HasComponent("CharacterController"))
			characterController = GetParent()->GetCharacterController();

		if (characterController != nullptr)
			maxSpeed = characterController->GetMaxSpeed();

		// Horizontal speed control
		if (velocity.x > maxSpeed)
			pendingForces.x -= equilibriumForce;
		else if (velocity.x < -maxSpeed)
			pendingForces.x += equilibriumForce;
		// Vertical speed control
		//if (velocity.y > maxSpeed)
		//	pendingForces.y -= equilibriumForce;
		//else if (velocity.y < -maxSpeed)
		//	pendingForces.y += equilibriumForce;
	}

	void RigidBody::ApplyCollisionForces()
	{
		std::shared_ptr<FlatEngine::BoxCollider> boxCollider = GetParent()->GetBoxCollider();
		std::shared_ptr<FlatEngine::Transform> transform = GetParent()->GetTransformComponent();
		Vector2 position = GetParent()->GetTransformComponent()->GetTruePosition();
		float activeHeight = boxCollider->GetActiveHeight();
		float activeWidth = boxCollider->GetActiveWidth();

		// Vertical Collision Forces
		// 
		// Normal Gravity
		if (gravity > 0 && _isGrounded && pendingForces.y < 0)
		{
			pendingForces.y = 0;
			float yPos = boxCollider->bottomCollision + (activeHeight / 2) - 0.001f;
			transform->SetPosition(Vector2(position.x, yPos));
		}
		// Inverted Gravity
		if (gravity < 0 && _isGrounded && pendingForces.y > 0)
		{
			pendingForces.y = 0;
			float yPos = boxCollider->topCollision + activeHeight / 2 - 0.001f;
			transform->SetPosition(Vector2(position.x, yPos));
		}

		// Ceilings
		if (gravity < 0 && pendingForces.y > 0 && boxCollider->_isCollidingTop)
		{
			pendingForces.y = 0;
			float yPos = boxCollider->topCollision + activeHeight / 2 + 0.001f;
			transform->SetPosition(Vector2(position.x, yPos));
		}

		// Horizontal Collision Forces
		// Collision on right side when moving to the right
		if (boxCollider->_isCollidingRight && boxCollider->_rightCollisionStatic && velocity.x > 0)
		{
		    pendingForces.x = 0;
			float xPos = boxCollider->rightCollision - activeWidth / 2 + 0.001f;
			transform->SetPosition(Vector2(xPos, position.y));
		}
		// Collision on left side when moving to the left
		if (boxCollider->_isCollidingLeft && boxCollider->_leftCollisionStatic && velocity.x < 0)
		{
			pendingForces.x = 0;
			float xPos = boxCollider->leftCollision + activeWidth / 2 - 0.001f;
			transform->SetPosition(Vector2(xPos, position.y));
		}
	}

	void RigidBody::AddForce(Vector2 direction, float power)
	{
		// Normalize the force first, then apply the power factor to the force
		Vector2 addedForce = Vector2(direction.x * power * forceCorrection, direction.y * power * forceCorrection);
		pendingForces.x += addedForce.x;
		pendingForces.y += addedForce.y;
	}

	Vector2 RigidBody::GetNextPosition()
	{
		Vector2 nextVelocity = Vector2(acceleration.x * GetDeltaTime(), acceleration.y * GetDeltaTime());
		std::shared_ptr<FlatEngine::Transform> transform = GetParent()->GetTransformComponent();
		Vector2 position = transform->GetPosition();
		return Vector2(position.x + nextVelocity.x * 2, position.y + nextVelocity.y * 2);
	}

	void RigidBody::Move(Vector2 position)
	{
		// At some point add some physics into this to make it actually move the character with force
		// and take into consideration weight, momentum, etc...
		std::shared_ptr<FlatEngine::Transform> transform = GetParent()->GetTransformComponent();
		transform->SetPosition(position);
	}

	void RigidBody::SetMass(float newMass)
	{
		mass = newMass;
	}

	float RigidBody::GetMass()
	{
		return mass;
	}

	void RigidBody::SetAngularDrag(float newAngularDrag)
	{
		angularDrag = newAngularDrag;
	}

	float RigidBody::GetAngularDrag()
	{
		return angularDrag;
	}

	void RigidBody::SetGravity(float newGravity)
	{
		gravity = newGravity;
	}

	float RigidBody::GetGravity()
	{
		return gravity;
	}

	void RigidBody::SetVelocity(Vector2 newVelocity)
	{
		velocity = newVelocity;
	}

	void RigidBody::SetTerminalVelocity(float newTerminalVelocity)
	{
		terminalVelocity = newTerminalVelocity;
	}

	float RigidBody::GetTerminalVelocity()
	{
		return terminalVelocity;
	}

	Vector2 RigidBody::GetVelocity()
	{
		return velocity;
	}

	Vector2 RigidBody::GetPendingForces()
	{
		return pendingForces;
	}

	void RigidBody::SetIsMoving(bool _moving)
	{
		_isMoving = _moving;
	}

	bool RigidBody::IsMoving()
	{
		return _isMoving;
	}

	void RigidBody::SetIsContinuous(bool _continuous)
	{
		_isContinious = _continuous;
	}

	bool RigidBody::IsContinuous()
	{
		return _isContinious;
	}

	void RigidBody::SetIsStatic(bool _static)
	{
		_isStatic = _static;
	}

	bool RigidBody::IsStatic()
	{
		return _isStatic;
	}

	void RigidBody::SetIsGrounded(bool _grounded)
	{
		_isGrounded = _grounded;
	}

	bool RigidBody::IsGrounded()
	{
		return _isGrounded;
	}

	void RigidBody::SetIsKinematic(bool _kinematic)
	{
		_isKinematic = _kinematic;
	}

	bool RigidBody::IsKinematic()
	{
		return _isKinematic;
	}
	
	void RigidBody::SetPendingForces(Vector2 newPendingForces)
	{
		pendingForces = newPendingForces;
	}

	float RigidBody::GetEquilibriumForce()
	{
		return equilibriumForce;
	}
}