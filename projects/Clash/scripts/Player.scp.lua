-- PlayerController.scp.lua


function Awake() 
    PlayerController[my_id] = 
    {
        gameScore = 0,
        scoreText = GetObjectByName("CurrentScore"),
        mappingContext = GetMappingContext("MC_Player"),
        characterController = this_object:GetCharacterController(),
        transform = this_object:GetTransform(),
        sprite = this_object:GetSprite(),
        rigidBody = this_object:GetRigidBody(),
        animation = this_object:GetAnimation(),
        boxCollider = this_object:GetBoxCollider(),
        hurtBox = this_object:FindChildByName("HurtBox"),
        lightAttackWindup = 50,
        lightAttackActiveTime = 200,
        lightAttackStartTime = 0,
        healthBar = GetObjectByName("CurrentHealth"),
        shootTimer = 80,
        lastShotTime = 0,
        totalHealth = 30,
        currentHealth = 30,
        powerUpStart = 0,
        powerUpMaxTime = 10000,
        poweredUp = false,
        b_hasDoubleJumped = false
    }    
end 

function Start()     
    -- local data = GetInstanceData("PlayerController", my_id)
    -- GetObjectByName("Music"):GetAudio():Play("ThemeMusic")
end 

function Update()     
    local data = GetInstanceData("PlayerController", my_id)
    handleMovement();
    handleAttacks();    

    if (data.rigidBody:IsGrounded()) then
        data.b_hasDoubleJumped = false
    end

    if GetTime() >= (data.powerUpStart + data.powerUpMaxTime) then          
        PowerDownPlayer(my_id)
    end
end 

function PowerUpPlayer(id)
    local data = GetInstanceData("PlayerController", id)

    data.powerUpStart = GetTime()
    data.shootTimer = 50
    data.poweredUp = true
end

function PowerDownPlayer(id)
    local data = GetInstanceData("PlayerController", id)

    data.shootTimer = 100
    data.poweredUp = false
end

function UpdatePlayerHealthbar(id)
    local data = GetInstanceData("PlayerController", id)

    local healthSprite = data.healthBar:GetSprite()
    local spriteScale = healthSprite:GetScale()
    local healthScale = data.currentHealth / data.totalHealth
    healthSprite:SetScale(Vector2:new(healthScale, spriteScale:y()))
end

function HealPlayer(id, amount)
    local data = GetInstanceData("PlayerController", id)

    if (data.currentHealth < data.totalHealth) then
        if (data.currentHealth + amount <= data.totalHealth) then
            data.currentHealth = data.currentHealth + 5
        else
            data.currentHealth = data.totalHealth
        end

        UpdatePlayerHealthbar(id)
    end
end

function DamagePlayer(id, amount)
    local data = GetInstanceData("PlayerController", id)

    GetObjectByName("Player"):GetAudio():Play("TakeHit")
    data.currentHealth = data.currentHealth - amount
    data.animation:Play("TakeDamage")
    UpdatePlayerHealthbar(id)

    if data.currentHealth <= 0 then
        data.boxCollider:SetActive(false)
        data.animation:Play("ShipExplode")     
        gameOverScreen = GetObjectByName("GameOverScreen")
        gameOverScreen:SetActive(true)
        -- GetObjectByName("Border"):SetActive(false)
        GetObjectByName("Music"):GetAudio():Stop("ThemeMusic")
        GetObjectByName("GlobalSounds"):GetAudio():Play("GameOver")
    end
end

function handleAttacks()
    local data = GetInstanceData("PlayerController", my_id)

    if data.mappingContext:ActionPressed("IA_LightAttack") then    
        data.lightAttackStartTime = GetTime()
    end

    if data.lightAttackStartTime ~= 0 and GetTime() > data.lightAttackStartTime + data.lightAttackWindup then
        data.hurtBox:SetActive(true)
    end

    if data.hurtBox:IsActive() then
        if GetTime() > data.lightAttackStartTime + data.lightAttackActiveTime then
            data.hurtBox:SetActive(false)
        end
    end
end

function handleMovement()
    local data = GetInstanceData("PlayerController", my_id)

    local b_hasLanded = false
    local b_movingLeft = false
    local b_movingRight = false
    local b_movingForward = false
    local b_movingBackward = false

    local moveDirection = Vector2:new(0,0)

    if data.mappingContext:Fired("IA_MoveForward") and (data.rigidBody:IsGrounded() or (data.b_hasDoubleJumped == false)) then
        local velocity = data.rigidBody:GetPendingForces()
        local newXVel = Vector2:new(0,0)

        if data.mappingContext:ActionPressed("IA_MoveRight") and velocity:x() < 0 then
            newXVel:SetX(0.3)
        elseif data.mappingContext:ActionPressed("IA_MoveLeft") and velocity:x() > 0 then
            newXVel:SetX(-0.3)
        else
            newXVel:SetX(velocity:x())
        end

        data.rigidBody:SetPendingForces(Vector2:new(newXVel:x(),0));


        data.rigidBody:AddForce(Vector2:new(0, 1), 30)
        
        if (data.b_hasDoubleJumped ~= true) then
            data.b_hasDoubleJumped = true
        end
    end

    if data.mappingContext:ActionPressed("IA_MoveRight") then        
        b_movingRight = true
        moveDirection:SetX(1)    
    end

    if data.mappingContext:ActionPressed("IA_MoveLeft") then
        b_movingLeft = true
        moveDirection:SetX(-1)
    end

    if b_movingLeft and b_movingRight then
        moveDirection = Vector2:new(0, moveDirection:y())
    end
    if b_movingBackward and b_movingForward then
        moveDirection = Vector2:new(moveDirection:x(), 0)
    end 

    if b_movingLeft or b_movingRight or b_movingBackward or b_movingForward then
        data.characterController:MoveToward(moveDirection)

    end
    
    local xVel = data.rigidBody:GetVelocity():x()
    if xVel < 0 then
        xVel = xVel * -1
    end

    data.sprite:SetScale(Vector2:new(1 - (xVel * 4), 1))
end

function OnBoxCollision(collidedWith)    
end

function OnBoxCollisionEnter(collidedWith)
    local collidedName = collidedWith:GetParent():GetName()       
end

function OnBoxCollisionLeave(collidedWith)    
end