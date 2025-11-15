-- ===================================
-- Screen Effects - Lua примеры
-- ===================================

-- ===================================
-- CAMERA SHAKE
-- ===================================

function OnPlayerHit(entity)
    local effects = entity:GetComponent("ScreenEffects")
    if effects then
        -- Слабая тряска при попадании
        effects:Shake(0.2, 5.0)  -- duration, intensity
    end
end

function OnExplosion(entity)
    local effects = entity:GetComponent("ScreenEffects")
    if effects then
        -- Сильная тряска при взрыве
        effects:Shake(0.8, 20.0, 25.0)  -- duration, intensity, frequency
    end
end

-- ===================================
-- SCREEN FLASH
-- ===================================

function OnDamageTaken(entity)
    local effects = entity:GetComponent("ScreenEffects")
    if effects then
        -- Красная вспышка
        effects:Flash(0.3, 1.0, 0.0, 0.0, 0.5)  -- duration, r, g, b, alpha
    end
end

function OnHeal(entity)
    local effects = entity:GetComponent("ScreenEffects")
    if effects then
        -- Зелёная вспышка
        effects:Flash(0.4, 0.0, 1.0, 0.0, 0.4)
    end
end

-- ===================================
-- SCREEN TRANSITIONS
-- ===================================

function OnPlayerDeath(entity)
    local effects = entity:GetComponent("ScreenEffects")
    if effects then
        -- Fade to black
        effects:FadeOut(1.0)
    end
end

function OnLevelStart(entity)
    local effects = entity:GetComponent("ScreenEffects")
    if effects then
        -- Fade in
        effects:FadeIn(1.5)
    end
end

-- ===================================
-- MOTION TRAIL
-- ===================================

function SetupPlayerTrail(player)
    local trail = player:GetComponent("Trail")
    if trail then
        -- Настроить trail
        trail:SetupTrail(
            0.5,   -- pointLifetime
            0.05,  -- emissionRate
            10.0,  -- startWidth
            2.0    -- endWidth
        )
        
        -- Включить trail при быстром движении
        local physics = player:GetComponent("Physics")
        if physics then
            local speed = physics.velocity:Length()
            if speed > 200 then
                trail:EnableTrail(true)
            else
                trail:EnableTrail(false)
            end
        end
    end
end

-- ===================================
-- DASH EFFECT
-- ===================================

function OnDash(player, direction)
    local trail = player:GetComponent("Trail")
    if trail then
        -- Настроить dash
        trail:SetupDash(0.3, 0.05, 10)  -- ghostLife, interval, maxGhosts
        
        -- Начать dash эффект
        trail:StartDash()
        
        -- Остановить через 0.3 сек
        -- Timer.DelayedCall(0.3, function()
        --     trail:StopDash()
        -- end)
    end
    
    -- Добавить screen effects
    local effects = player:GetComponent("ScreenEffects")
    if effects then
        effects:Flash(0.15, 0.3, 0.5, 1.0, 0.4)  -- Синяя вспышка
        effects:Shake(0.2, 3.0, 30.0)             -- Лёгкая тряска
    end
end

-- ===================================
-- КОМБО ЭФФЕКТОВ
-- ===================================

function OnCriticalHit(entity)
    local effects = entity:GetComponent("ScreenEffects")
    if effects then
        -- Жёлтая вспышка + тряска
        effects:Flash(0.2, 1.0, 1.0, 0.0, 0.6)
        effects:Shake(0.3, 10.0)
    end
end

function OnBossDeath(entity)
    local effects = entity:GetComponent("ScreenEffects")
    if effects then
        -- 1. Белая вспышка
        effects:Flash(0.5, 1.0, 1.0, 1.0, 1.0)
        
        -- 2. Сильная тряска
        effects:Shake(2.0, 25.0, 15.0)
        
        -- 3. Fade out через 2.5 сек
        -- Timer.DelayedCall(2.5, function()
        --     effects:FadeOut(1.0)
        -- end)
    end
end
