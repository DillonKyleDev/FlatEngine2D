-- Spawner.scp.lua
-- use "this_object" to reference the object that owns this script and "my_id" to access it's id


function Awake() 
     Spawner[my_id] =
     {
		spawnedParticles = 0,
          lastParticleSpawned = GetTime(),
          transform = this_object:GetTransform()
     }
     local data = GetInstanceData("Spawner", my_id)
end

function Start()
     -- required to access instance data
     local data = GetInstanceData("Spawner", my_id)
end

function Update()
     local data = GetInstanceData("Spawner", my_id)

     local maxParticles = GetScriptParam("maxParticles", my_id):int()
     local spawnRate = GetScriptParam("spawnRate", my_id):int()

     if (data.lastParticleSpawned + spawnRate < GetTime() and data.spawnedParticles < maxParticles) then
          data.lastParticleSpawned = GetTime()
          local particle = Instantiate("Particle", data.transform:GetPosition())
          local body = particle:GetBody()
          local sprite = particle:GetSprite():SetTintColor(Vector4:new(RandomFloat(0.0, 1.0),RandomFloat(0.0, 1.0),RandomFloat(0.0, 1.0),1))
          body:ApplyForceToCenter(Vector2:new(RandomFloat(-100, 100), RandomFloat(-100, 100)))
     end
end

-- each of these functions must be present in each Lua script file otherwise other script's implementations will be used with this object instead
function OnBeginCollision(collidedWith, manifold)
     local data = GetInstanceData("Spawner", my_id)
end

function OnEndCollision(collidedWith, manifold)
     local data = GetInstanceData("Spawner", my_id)
end

function OnBeginSensorTouch(touched)
     local data = GetInstanceData("Spawner", my_id)
end

function OnEndSensorTouch(touched)
     local data = GetInstanceData("Spawner", my_id)
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