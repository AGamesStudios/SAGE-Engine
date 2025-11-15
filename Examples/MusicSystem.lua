-- SAGE Music System - Lua Example
print("=== SAGE Music System - Lua Example ===")

-- Получить музыкальную систему
local musicSystem = SAGE.GetMusicSystem()
local soundVariations = SAGE.GetSoundVariationSystem()

-- ===============================
-- 1. БАЗОВАЯ МУЗЫКА
-- ===============================
print("\n1. Basic Music Playback")

musicSystem:RegisterTrack("menu_music", "assets/music/menu.ogg", true)
musicSystem:RegisterTrack("game_music", "assets/music/game.ogg", true)
musicSystem:RegisterTrack("boss_music", "assets/music/boss.ogg", true)

-- Воспроизвести с фейдом
musicSystem:Play("menu_music", 2.0)
print("Playing menu music with 2s fade-in")

-- ===============================
-- 2. КРОССФЕЙД
-- ===============================
print("\n2. Crossfading")

-- Перейти к игровой музыке
function StartGame()
    musicSystem:CrossfadeToTrack("game_music", 3.0)
    print("Crossfading to game music over 3 seconds")
end

-- ===============================
-- 3. АДАПТИВНАЯ МУЗЫКА
-- ===============================
print("\n3. Adaptive Music with Layers")

musicSystem:RegisterTrackWithLayers("combat_music", 
    "assets/music/combat_base.ogg",
    {
        "assets/music/combat_percussion.ogg",
        "assets/music/combat_melody.ogg",
        "assets/music/combat_choir.ogg"
    }
)

function StartCombat()
    musicSystem:Play("combat_music")
    print("Combat started - playing base layer")
end

function IncreaseCombatIntensity(intensity)
    if intensity >= 0.3 then
        musicSystem:FadeInLayer("combat_music", "combat_music_layer_0", 1.5)
        print("Adding percussion layer")
    end
    
    if intensity >= 0.6 then
        musicSystem:FadeInLayer("combat_music", "combat_music_layer_1", 1.5)
        print("Adding melody layer")
    end
    
    if intensity >= 0.9 then
        musicSystem:FadeInLayer("combat_music", "combat_music_layer_2", 1.5)
        print("Adding choir layer - EPIC!")
    end
end

function DecreaseCombatIntensity()
    musicSystem:FadeOutLayer("combat_music", "combat_music_layer_2", 1.0)
    musicSystem:FadeOutLayer("combat_music", "combat_music_layer_1", 1.5)
    musicSystem:FadeOutLayer("combat_music", "combat_music_layer_0", 2.0)
    print("Combat ending - fading out intensity")
end

-- ===============================
-- 4. ПЛЕЙЛИСТЫ
-- ===============================
print("\n4. Playlists")

musicSystem:CreatePlaylist("exploration", {
    "forest_1", "forest_2", "cave_1", "mountain_1"
}, false, true)  -- no shuffle, loop

musicSystem:CreatePlaylist("boss_phases", {
    "boss_phase1", "boss_phase2", "boss_phase3"
}, false, false)  -- sequential, no loop

function PlayExplorationMusic()
    musicSystem:PlayPlaylist("exploration", 2.0)
    print("Playing exploration playlist")
end

function NextSong()
    musicSystem:NextTrack(1.5)
end

function PreviousSong()
    musicSystem:PreviousTrack(1.5)
end

-- ===============================
-- 5. ВАРИАЦИИ ЗВУКОВ
-- ===============================
print("\n5. Sound Variations")

-- Регистрация вариаций для выстрелов
soundVariations:RegisterVariation("pistol_shot", {
    "assets/sounds/pistol1.ogg",
    "assets/sounds/pistol2.ogg",
    "assets/sounds/pistol3.ogg"
})

soundVariations:SetPitchRange("pistol_shot", 0.9, 1.1)
soundVariations:SetVolumeRange("pistol_shot", 0.85, 1.0)

-- Регистрация вариаций для шагов
soundVariations:RegisterVariation("metal_footstep", {
    "assets/sounds/metal_step1.ogg",
    "assets/sounds/metal_step2.ogg",
    "assets/sounds/metal_step3.ogg",
    "assets/sounds/metal_step4.ogg"
})

soundVariations:SetPitchRange("metal_footstep", 0.95, 1.05)
soundVariations:SetVolumeRange("metal_footstep", 0.9, 1.0)

-- Регистрация вариаций для попаданий
soundVariations:RegisterVariation("impact", {
    "assets/sounds/impact1.ogg",
    "assets/sounds/impact2.ogg",
    "assets/sounds/impact3.ogg",
    "assets/sounds/impact4.ogg",
    "assets/sounds/impact5.ogg"
})

soundVariations:SetPitchRange("impact", 0.8, 1.2)
soundVariations:SetVolumeRange("impact", 0.8, 1.0)

-- Функции для игровых событий
function OnPlayerShoot()
    soundVariations:PlayVariation("pistol_shot")
end

function OnPlayerWalk()
    soundVariations:PlayVariation("metal_footstep")
end

function OnEnemyHit()
    soundVariations:PlayVariation("impact")
end

-- ===============================
-- 6. CALLBACKS
-- ===============================
print("\n6. Music Callbacks")

musicSystem:SetOnTrackStart(function(trackId)
    print("Track started: " .. trackId)
    
    -- Пример: показать название трека в UI
    if trackId == "boss_music" then
        print("!!! BOSS FIGHT !!!")
    end
end)

musicSystem:SetOnTrackEnd(function(trackId)
    print("Track ended: " .. trackId)
end)

musicSystem:SetOnCrossfadeComplete(function(fromTrack, toTrack)
    print("Crossfade complete: " .. fromTrack .. " -> " .. toTrack)
end)

-- ===============================
-- 7. УПРАВЛЕНИЕ ГРОМКОСТЬЮ
-- ===============================
print("\n7. Volume Control")

function SetMusicVolume(volume)
    musicSystem:SetMasterVolume(volume)
    print("Music volume set to: " .. volume)
end

function SetTrackVolume(trackId, volume)
    musicSystem:SetTrackVolume(trackId, volume)
    print("Track " .. trackId .. " volume set to: " .. volume)
end

-- ===============================
-- 8. ИГРОВЫЕ ПРИМЕРЫ
-- ===============================
print("\n8. Game Scenario Examples")

-- Пример: адаптивная музыка на основе количества врагов
local enemyCount = 0

function OnEnemySpawned()
    enemyCount = enemyCount + 1
    local intensity = math.min(enemyCount / 10.0, 1.0)
    IncreaseCombatIntensity(intensity)
end

function OnEnemyKilled()
    enemyCount = math.max(0, enemyCount - 1)
    
    if enemyCount == 0 then
        DecreaseCombatIntensity()
        -- Вернуться к спокойной музыке
        musicSystem:CrossfadeToTrack("game_music", 3.0)
    end
end

-- Пример: музыка на основе здоровья игрока
function UpdateMusicByHealth(healthPercent)
    if healthPercent <= 20 then
        -- Критическое здоровье - напряжённая музыка
        musicSystem:FadeInLayer("combat_music", "combat_music_layer_2", 0.5)
        musicSystem:SetLayerVolume("combat_music", "combat_music_layer_2", 1.5)
    elseif healthPercent <= 50 then
        -- Среднее здоровье
        musicSystem:FadeOutLayer("combat_music", "combat_music_layer_2", 1.0)
        musicSystem:FadeInLayer("combat_music", "combat_music_layer_1", 0.5)
    else
        -- Нормальное здоровье
        musicSystem:FadeOutLayer("combat_music", "combat_music_layer_2", 1.0)
        musicSystem:FadeOutLayer("combat_music", "combat_music_layer_1", 1.0)
    end
end

-- Пример: музыка для разных биомов
function OnEnterBiome(biomeName)
    if biomeName == "forest" then
        musicSystem:CrossfadeToTrack("forest_music", 2.0)
    elseif biomeName == "cave" then
        musicSystem:CrossfadeToTrack("cave_music", 2.0)
    elseif biomeName == "dungeon" then
        musicSystem:CrossfadeToTrack("dungeon_music", 2.0)
    end
end

-- Пример: музыка босса с фазами
local bossPhase = 1

function OnBossPhaseChange(phase)
    bossPhase = phase
    
    if phase == 1 then
        musicSystem:Play("boss_music")
    elseif phase == 2 then
        musicSystem:FadeInLayer("boss_music", "boss_music_layer_0", 1.0)
        print("Boss Phase 2 - Music intensifies!")
    elseif phase == 3 then
        musicSystem:FadeInLayer("boss_music", "boss_music_layer_1", 1.0)
        musicSystem:FadeInLayer("boss_music", "boss_music_layer_2", 1.0)
        print("Boss Phase 3 - FINAL PHASE!")
    end
end

function OnBossDefeated()
    musicSystem:Stop("boss_music", 2.0)
    musicSystem:Play("victory_music", 1.0)
    print("Victory!")
end

-- ===============================
-- 9. ДИАГНОСТИКА
-- ===============================
print("\n9. Music System Status")

function PrintMusicStatus()
    local currentTrack = musicSystem:GetCurrentTrack()
    local isPlaying = musicSystem:IsPlaying(currentTrack)
    local state = musicSystem:GetTrackState(currentTrack)
    
    print("Current track: " .. (currentTrack or "none"))
    print("Is playing: " .. tostring(isPlaying))
    print("State: " .. tostring(state))
end

print("\n=== Music System Ready ===")
print("Call StartGame() to begin!")
