-- Wheel.scp.lua
-- use "this_object" to reference the object that owns this script and "my_id" to access it's id


function Awake() 
     Wheel[my_id] =
     {
		frontWheel = GetObjectByName("FrontWheel"),
          backWheel = GetObjectByName("BackWheel"),
          jointMaker = GetObjectByName("JointMaker"),
          mappingContext = GetMappingContext("MC_Player"),
          motorVelocity = 0,
          motorSpeed = 0,
     }
     local data = GetInstanceData("Wheel", my_id)
end

function Start()
     -- required to access instance data
     local data = GetInstanceData("Wheel", my_id)
     data["frontWheelJoint"] = data.jointMaker:GetJointMaker():GetWheelJointByBodyID(data.frontWheel:GetID())
     data["backWheelJoint"] = data.jointMaker:GetJointMaker():GetWheelJointByBodyID(data.backWheel:GetID())
end

function Update()
     local data = GetInstanceData("Wheel", my_id)

     if data.mappingContext:ActionPressed("IA_Left") then
          data.motorVelocity = data.motorVelocity + .1                
     end
     if data.mappingContext:ActionPressed("IA_Right") then
          data.motorVelocity = data.motorVelocity - .1          
     end

     local newSpeed = data.frontWheelJoint:GetMotorSpeed()

     if (data.motorSpeed <= 10) then
          newSpeed = newSpeed + data.motorVelocity          
     end

     data.motorVelocity = data.motorVelocity * 0.99
     newSpeed = newSpeed * 0.9

     data.frontWheelJoint:SetMotorSpeed(data.motorSpeed + data.motorVelocity)
     data.backWheelJoint:SetMotorSpeed(data.motorSpeed + data.motorVelocity)
end

-- each of these functions must be present in each Lua script file otherwise other script's implementations will be used with this object instead
function OnBeginCollision(collidedWith, manifold)
     local data = GetInstanceData("Wheel", my_id)
end

function OnEndCollision(collidedWith, manifold)
     local data = GetInstanceData("Wheel", my_id)
end

function OnBeginSensorTouch(touched)
     local data = GetInstanceData("Wheel", my_id)
end

function OnEndSensorTouch(touched)
     local data = GetInstanceData("Wheel", my_id)
end

function OnButtonMouseOver()
end

function OnButtonMouseEnter()
end

function OnButtonMouseLeave()
end

function OnButtonLeftClick()
end

function OnButtonRightClick()
end


--Lua cheatsheet

--Lua if statements:
--if (test) then
-- <do something>
--elseif
-- <do something>
--end

--Lua for loops:
--for init, min/max value, increment
--do
-- <do something>
--end


--example:
--for i = 0, 10, 1
--do
--LogInt(i)
--end