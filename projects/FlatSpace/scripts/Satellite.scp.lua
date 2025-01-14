-- Satellite.scp.lua

-- Use "this_object" to reference the object that owns this script and "my_id" to access its id


function Awake() 
     Satellite[my_id] =
     {
		thisObject = this_object,
          transform = this_object:GetTransform(),
          animation = this_object:GetAnimation(),
          boxCollider = this_object:GetBoxCollider(),
          currentHealth = GetScriptParam("health", my_id):int(),
          totalHealth = GetScriptParam("health", my_id):int(),
          startTime = GetTime(),
          lifeTime = 30000
     }
     local data = GetInstanceData("Satellite", my_id)
     data.animation:Play("Blink")
end

function Start()
     -- required to access instance data
     local data = GetInstanceData("Satellite", my_id)
end

function Update()
     local data = GetInstanceData("Satellite", my_id)
     local rotation = data.transform:GetRotation()
     data.transform:SetRotation(rotation + GetScriptParam("rotationSpeed", my_id):float())

     if GetTime() >= (data.startTime + data.lifeTime) then          
          Destroy(my_id)
     end
end

function SetSatelliteLifeTime(id, lifeTime)
     local data = GetInstanceData("Satellite", id)

     data.lifeTime = lifeTime
end

function DamageSatellite(id, amount)     
     local data = GetInstanceData("Satellite", id)
     data.currentHealth = data.currentHealth - amount

     if data.currentHealth <= 0 then
          GetObjectByID(id):GetRigidBody():SetPendingForces(Vector2:new(0,0))
          data.boxCollider:SetActive(false)
          data.animation:Play("Explode")   
          AddToScore(50)    
     
          local dropChance = RandomInt(0, 10)          
          local whichDrop = RandomInt(0, 10) 
          local randFloatX = RandomFloat(-1, 1)
          local randFloatY = RandomFloat(-1, 1)          

          if (dropChance < 6) then
               if (whichDrop < 4) then
                    local healthPack = Instantiate("HealthPack", data.transform:GetTruePosition())
                    healthPack:GetRigidBody():AddForce(Vector2:new(randFloatX, randFloatY), 1)
               else
                    local powerUp = Instantiate("PowerUp", data.transform:GetTruePosition())
                    powerUp:GetRigidBody():AddForce(Vector2:new(randFloatX, randFloatY), 1)
               end
          end
     end
end

-- each of these functions must be present in each file if they are to be called otherwise other scripts copies will be used with this object instead
function OnBoxCollision(collidedWith)
end

function OnBoxCollisionEnter(collidedWith)
     local data = GetInstanceData("Satellite", my_id)

     if (collidedWith:GetParent():HasTag("Player")) then          
          DamagePlayer(collidedWith:GetParentID(), 5)
     end
end

function OnBoxCollisionLeave(collidedWith)
end

function OnCircleCollision(collidedWith)
end

function OnCircleCollisionEnter(collidedWith)
end

function OnCircleCollisionLeave(collidedWith)
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