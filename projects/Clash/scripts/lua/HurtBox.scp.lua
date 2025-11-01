-- HurtBox.scp.lua
-- use "this_object" to reference the object that owns this script and "my_id" to access it's id


function Awake() 
     HurtBox[my_id] =
     {
		-- Key value pairs here
     }
     local data = GetInstanceData("HurtBox", my_id)
end

function Start()
     -- required to access instance data
     local data = GetInstanceData("HurtBox", my_id)
end

function Update()
     local data = GetInstanceData("HurtBox", my_id)
end

-- each of these functions must be present in each Lua script file otherwise other script's implementations will be used with this object instead
function OnBeginCollision(collidedWith, manifold)
     local data = GetInstanceData("HurtBox", my_id)
end

function OnEndCollision(collidedWith, manifold)
     local data = GetInstanceData("HurtBox", my_id)
end

function OnBeginSensorTouch(touched)
     local data = GetInstanceData("HurtBox", my_id)

     touched:GetParent():GetBody():ApplyForceToCenter(Vector2:new(3000, 3000))
end

function OnEndSensorTouch(touched)
     local data = GetInstanceData("HurtBox", my_id)
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