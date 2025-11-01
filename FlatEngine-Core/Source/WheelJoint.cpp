#include "WheelJoint.h"
#include "Body.h"



namespace FlatEngine
{
	WheelJoint::WheelJoint(BaseProps baseProps, WheelJointProps jointProps) : Joint(baseProps)
	{
		m_jointProps = jointProps;
		m_jointString = "Wheel Joint";
		m_baseProps.jointType = JT_Wheel;		
	}

	WheelJoint::~WheelJoint()
	{
	}

	json WheelJoint::GetJointData()
	{
		json jointData = json::object();

		json jsonData = {
			{ "dampingRatio", m_jointProps.dampingRatio },
			{ "_enableLimit", m_jointProps.b_enableLimit },
			{ "_enableMotor", m_jointProps.b_enableMotor },
			{ "_enableSpring", m_jointProps.b_enableSpring },
			{ "hertz", m_jointProps.hertz },
			{ "localAxisAX", m_jointProps.localAxisA.x },
			{ "localAxisAY", m_jointProps.localAxisA.y },
			{ "lowerTranslation", m_jointProps.lowerTranslation },
			{ "upperTranslation", m_jointProps.upperTranslation },
			{ "maxMotorTorque", m_jointProps.maxMotorTorque },
			{ "motorSpeed", m_jointProps.motorSpeed }			
		};

		jointData.emplace("baseProps", GetBasePropsData());
		jointData.emplace("jointProps", jsonData);

		return jointData;
	}

	WheelJoint::WheelJointProps& WheelJoint::GetJointProps()
	{
		return m_jointProps;
	}

	void WheelJoint::SetJointProps(WheelJointProps jointProps)
	{
		m_jointProps = jointProps;
	}

	void WheelJoint::SetEnableSpring(bool b_enableSpring)
	{
		m_jointProps.b_enableSpring = b_enableSpring;

		if (b2Joint_IsValid(m_b2JointID))
		{
			b2WheelJoint_EnableSpring(m_b2JointID, b_enableSpring);
		}
		else
		{
			CreateJoint();
		}
	}

	void WheelJoint::SetSpringHertz(float springHertz)
	{
		if (springHertz >= 0)
		{
			m_jointProps.hertz = springHertz;

			if (b2Joint_IsValid(m_b2JointID))
			{
				b2WheelJoint_SetSpringHertz(m_b2JointID, springHertz);
			}
			else
			{
				CreateJoint();
			}
		}
	}

	float WheelJoint::GetSpringHertz()
	{
		if (b2Joint_IsValid(m_b2JointID))
		{
			return b2WheelJoint_GetSpringHertz(m_b2JointID);
		}
		else
		{
			return m_jointProps.hertz;
		}
	}

	void WheelJoint::SetSpringDampingRatio(float springDampingRatio)
	{
		if (springDampingRatio >= 0)
		{
			m_jointProps.dampingRatio = springDampingRatio;

			if (b2Joint_IsValid(m_b2JointID))
			{
				b2WheelJoint_SetSpringDampingRatio(m_b2JointID, springDampingRatio);
			}
			else
			{
				CreateJoint();
			}
		}
	}

	float WheelJoint::GetSpringDampingRatio()
	{
		if (b2Joint_IsValid(m_b2JointID))
		{
			return b2WheelJoint_GetSpringDampingRatio(m_b2JointID);
		}
		else
		{
			return m_jointProps.dampingRatio;
		}
	}

	void WheelJoint::SetEnableMotor(bool b_enableMotor)
	{
		m_jointProps.b_enableMotor = b_enableMotor;

		if (b2Joint_IsValid(m_b2JointID))
		{
			b2WheelJoint_EnableMotor(m_b2JointID, b_enableMotor);
		}
		else
		{
			CreateJoint();
		}
	}

	void WheelJoint::SetMotorSpeed(float motorSpeed)
	{
		//if (m_jointProps.motorSpeed == 0)
		{
			b2Joint_WakeBodies(m_b2JointID);
		}

		m_jointProps.motorSpeed = motorSpeed;

		if (b2Joint_IsValid(m_b2JointID))
		{
			b2WheelJoint_SetMotorSpeed(m_b2JointID, motorSpeed);
		}
		else
		{
			CreateJoint();
		}
	}

	float WheelJoint::GetMotorSpeed()
	{
		if (b2Joint_IsValid(m_b2JointID))
		{
			return b2WheelJoint_GetMotorSpeed(m_b2JointID);
		}
		else
		{
			return m_jointProps.motorSpeed;
		}
	}

	void WheelJoint::SetMaxMotorTorque(float maxMotorForce)
	{
		if (maxMotorForce >= 0)
		{
			m_jointProps.maxMotorTorque = maxMotorForce;

			if (b2Joint_IsValid(m_b2JointID))
			{
				b2WheelJoint_SetMaxMotorTorque(m_b2JointID, maxMotorForce);
			}
			else
			{
				CreateJoint();
			}
		}
	}

	float WheelJoint::GetMotorTorque()
	{
		if (b2Joint_IsValid(m_b2JointID))
		{
			return b2WheelJoint_GetMaxMotorTorque(m_b2JointID);
		}
		else
		{
			return 0;
		}
	}

	void WheelJoint::SetEnableLimit(bool b_enableLimit)
	{
		m_jointProps.b_enableLimit = b_enableLimit;

		if (b2Joint_IsValid(m_b2JointID))
		{
			b2WheelJoint_EnableLimit(m_b2JointID, b_enableLimit);
		}
		else
		{
			CreateJoint();
		}
	}
}