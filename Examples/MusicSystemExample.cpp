#include "../Engine/Audio/MusicSystem.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace SAGE;

int main() {
    std::cout << "=== SAGE Music System Example ===" << std::endl;
    
    // Создать системы
    MusicSystem musicSystem;
    SoundVariationSystem soundVariations;
    
    // === 1. БАЗОВАЯ МУЗЫКА ===
    std::cout << "\n1. Базовая музыка" << std::endl;
    
    musicSystem.RegisterTrack("menu_music", "assets/music/menu.ogg", true);
    musicSystem.RegisterTrack("battle_music", "assets/music/battle.ogg", true);
    musicSystem.RegisterTrack("victory_music", "assets/music/victory.ogg", false);
    
    // Воспроизвести музыку меню с фейдом
    musicSystem.Play("menu_music", 2.0f);
    std::cout << "Playing menu music with 2s fade-in" << std::endl;
    
    // === 2. КРОССФЕЙД ===
    std::cout << "\n2. Кроссфейд между треками" << std::endl;
    
    // Через 5 секунд перейти к боевой музыке
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    musicSystem.Crossfade("menu_music", "battle_music", 3.0f);
    std::cout << "Crossfading to battle music over 3 seconds" << std::endl;
    
    // === 3. АДАПТИВНАЯ МУЗЫКА С СЛОЯМИ ===
    std::cout << "\n3. Адаптивная музыка (слои)" << std::endl;
    
    musicSystem.RegisterTrackWithLayers("adaptive_battle", 
        "assets/music/battle_base.ogg",
        {
            "assets/music/battle_drums.ogg",
            "assets/music/battle_brass.ogg",
            "assets/music/battle_strings.ogg"
        }
    );
    
    musicSystem.Play("adaptive_battle");
    std::cout << "Playing adaptive battle music (base layer)" << std::endl;
    
    // Добавить барабаны через 3 секунды
    std::this_thread::sleep_for(std::chrono::seconds(3));
    musicSystem.FadeInLayer("adaptive_battle", "adaptive_battle_layer_0", 2.0f);
    std::cout << "Adding drums layer" << std::endl;
    
    // Добавить духовые через 2 секунды
    std::this_thread::sleep_for(std::chrono::seconds(2));
    musicSystem.FadeInLayer("adaptive_battle", "adaptive_battle_layer_1", 2.0f);
    std::cout << "Adding brass layer" << std::endl;
    
    // Добавить струнные через 2 секунды
    std::this_thread::sleep_for(std::chrono::seconds(2));
    musicSystem.FadeInLayer("adaptive_battle", "adaptive_battle_layer_2", 2.0f);
    std::cout << "Adding strings layer - full intensity!" << std::endl;
    
    // Убрать слои при победе
    std::this_thread::sleep_for(std::chrono::seconds(5));
    musicSystem.FadeOutLayer("adaptive_battle", "adaptive_battle_layer_0", 1.0f);
    musicSystem.FadeOutLayer("adaptive_battle", "adaptive_battle_layer_1", 1.0f);
    musicSystem.FadeOutLayer("adaptive_battle", "adaptive_battle_layer_2", 1.0f);
    std::cout << "Victory! Fading out intensity layers" << std::endl;
    
    // === 4. ПЛЕЙЛИСТЫ ===
    std::cout << "\n4. Плейлисты" << std::endl;
    
    musicSystem.CreatePlaylist("ambient_playlist", {
        "ambient_1", "ambient_2", "ambient_3", "ambient_4"
    }, false, true);
    
    musicSystem.PlayPlaylist("ambient_playlist", 2.0f);
    std::cout << "Playing ambient playlist with crossfade" << std::endl;
    
    // Перейти к следующему треку
    std::this_thread::sleep_for(std::chrono::seconds(10));
    musicSystem.NextTrack(2.0f);
    std::cout << "Next track" << std::endl;
    
    // === 5. ВАРИАЦИИ ЗВУКОВ ===
    std::cout << "\n5. Вариации звуков" << std::endl;
    
    // Зарегистрировать вариации выстрелов
    soundVariations.RegisterVariation("gun_shot", {
        "assets/sounds/shot1.ogg",
        "assets/sounds/shot2.ogg",
        "assets/sounds/shot3.ogg",
        "assets/sounds/shot4.ogg"
    });
    
    soundVariations.SetPitchRange("gun_shot", 0.9f, 1.1f);
    soundVariations.SetVolumeRange("gun_shot", 0.85f, 1.0f);
    
    // Зарегистрировать вариации шагов
    soundVariations.RegisterVariation("footstep", {
        "assets/sounds/step1.ogg",
        "assets/sounds/step2.ogg",
        "assets/sounds/step3.ogg",
        "assets/sounds/step4.ogg",
        "assets/sounds/step5.ogg"
    });
    
    soundVariations.SetPitchRange("footstep", 0.95f, 1.05f);
    soundVariations.SetVolumeRange("footstep", 0.9f, 1.0f);
    
    // Callback для воспроизведения
    soundVariations.SetPlayCallback([](const std::string& file, float pitch, float volume) {
        std::cout << "Playing sound: " << file 
                  << " (pitch: " << pitch 
                  << ", volume: " << volume << ")" << std::endl;
    });
    
    // Воспроизвести несколько выстрелов
    std::cout << "\nFiring shots with variations:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        soundVariations.PlayVariation("gun_shot");
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    
    // Воспроизвести шаги
    std::cout << "\nWalking with footstep variations:" << std::endl;
    for (int i = 0; i < 8; ++i) {
        soundVariations.PlayVariation("footstep");
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
    
    // === 6. CALLBACKS ===
    std::cout << "\n6. Callbacks" << std::endl;
    
    musicSystem.SetOnTrackStart([](const std::string& trackId) {
        std::cout << "Track started: " << trackId << std::endl;
    });
    
    musicSystem.SetOnTrackEnd([](const std::string& trackId) {
        std::cout << "Track ended: " << trackId << std::endl;
    });
    
    musicSystem.SetOnCrossfadeComplete([](const std::string& from, const std::string& to) {
        std::cout << "Crossfade complete: " << from << " -> " << to << std::endl;
    });
    
    // === 7. ПРИМЕР ИГРОВОГО ЦИКЛА ===
    std::cout << "\n7. Симуляция игрового цикла" << std::endl;
    
    float deltaTime = 0.016f;  // ~60 FPS
    
    for (int frame = 0; frame < 60; ++frame) {
        musicSystem.Update(deltaTime);
        
        if (frame % 30 == 0) {
            std::cout << "Frame " << frame << " - updating music system" << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    // === 8. ДИНАМИЧЕСКИЙ МИКС НА ОСНОВЕ ЗДОРОВЬЯ ===
    std::cout << "\n8. Динамический микс (симуляция изменения HP)" << std::endl;
    
    musicSystem.Play("adaptive_battle");
    
    float playerHealth = 100.0f;
    
    for (int i = 0; i < 10; ++i) {
        playerHealth -= 10.0f;
        
        std::cout << "Player health: " << playerHealth << "%" << std::endl;
        
        // При низком здоровье убрать слои, оставить только напряжённые
        if (playerHealth <= 30.0f) {
            musicSystem.SetLayerActive("adaptive_battle", "adaptive_battle_layer_0", false, 1.0f);
            musicSystem.SetLayerActive("adaptive_battle", "adaptive_battle_layer_1", true, 1.0f);
            std::cout << "Low health - intense music!" << std::endl;
        }
        else if (playerHealth <= 60.0f) {
            musicSystem.SetLayerActive("adaptive_battle", "adaptive_battle_layer_0", true, 1.0f);
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "\n=== Example Complete ===" << std::endl;
    
    return 0;
}
