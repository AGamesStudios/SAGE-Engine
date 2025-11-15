#pragma once

#include "Audio/AudioSystem.h"
#include "Math/Vector3.h"
#include <string>

namespace SAGE::ECS {

/**
 * @brief Универсальный компонент аудио (источник + слушатель)
 * 
 * Если isListener = true, то это AudioListener (камера/игрок)
 * Иначе это AudioSource (звук от объекта)
 */
struct AudioComponent {
    // === Режим работы ===
    bool isListener = false;  ///< True = AudioListener, False = AudioSource
    bool active = true;       ///< Активен ли компонент
    
    // === Для AudioSource (isListener = false) ===
    std::string soundName;                   ///< Название звука
    float volume = 1.0f;                     ///< Громкость (0-1)
    float pitch = 1.0f;                      ///< Высота звука
    float pan = 0.0f;                        ///< Панорамирование (-1 левый, +1 правый)
    bool spatial = false;                    ///< 3D позиционирование
    bool looping = false;                    ///< Зацикленный звук
    bool playOnStart = false;                ///< Играть при создании
    bool streaming = false;                  ///< Потоковое воспроизведение
    AudioCategory category = AudioCategory::SFX;  ///< Категория (SFX/Music/Voice)
    
    // Пространственные настройки
    AttenuationSettings attenuation{};       ///< Затухание на расстоянии
    ReverbSettings reverb{};                 ///< Реверберация
    OcclusionSettings occlusion{};           ///< Перекрытие звука
    bool useDoppler = false;                 ///< Эффект Доплера
    float dopplerFactor = 1.0f;              ///< Множитель Доплера
    
    // Отслеживание движения
    bool trackVelocity = true;               ///< Отслеживать скорость для Доплера
    ::SAGE::Vector3 offset{};                ///< Смещение от Transform
    ::SAGE::Vector3 velocity{};              ///< Скорость движения
    ::SAGE::Vector3 lastWorldPosition{};     ///< Последняя позиция
    bool hasLastWorldPosition = false;       ///< Есть ли lastWorldPosition
    
    // Состояние воспроизведения (только для Source)
    AudioHandle handle{};                    ///< Handle звука в аудиосистеме
    bool playRequested = false;              ///< Запрос на воспроизведение
    bool stopRequested = false;              ///< Запрос на остановку
    bool hasStarted = false;                 ///< Звук уже начал играть
    bool isPlaying = false;                  ///< Звук сейчас играет

    AudioComponent() = default;
    
    /// \brief Создать AudioListener
    static AudioComponent CreateListener(bool activeState = true) {
        AudioComponent audio;
        audio.isListener = true;
        audio.active = activeState;
        return audio;
    }
    
    /// \brief Создать AudioSource
    static AudioComponent CreateSource(const std::string& sound, float vol = 1.0f, 
                                       bool loop = false, bool spatialAudio = false) {
        AudioComponent audio;
        audio.isListener = false;
        audio.soundName = sound;
        audio.volume = vol;
        audio.looping = loop;
        audio.spatial = spatialAudio;
        return audio;
    }
    
    /// \brief Проверить, является ли слушателем
    bool IsListener() const { return isListener; }
    
    /// \brief Проверить, является ли источником
    bool IsSource() const { return !isListener; }
};

} // namespace SAGE::ECS
