#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

namespace SAGE {

// Состояние музыки
enum class MusicState {
    Stopped,
    Playing,
    Paused,
    FadingIn,
    FadingOut,
    Crossfading
};

// Музыкальный трек
struct MusicTrack {
    std::string id;
    std::string filepath;
    float volume = 1.0f;
    float defaultVolume = 1.0f;
    bool loop = true;
    
    // Текущее состояние
    MusicState state = MusicState::Stopped;
    float currentTime = 0.0f;
    float duration = 0.0f;
    
    // Фейд
    float fadeVolume = 0.0f;
    float fadeSpeed = 1.0f;  // Единиц громкости в секунду
    float targetVolume = 1.0f;
    
    // Адаптивная музыка
    std::vector<std::string> layers;  // ID слоев для адаптивной музыки
    std::unordered_map<std::string, float> layerVolumes;  // Громкость каждого слоя
};

// Музыкальный слой (для адаптивной музыки)
struct MusicLayer {
    std::string id;
    std::string filepath;
    float volume = 1.0f;
    bool active = false;
    
    float fadeVolume = 0.0f;
    float fadeSpeed = 1.0f;
    float targetVolume = 1.0f;
};

// Плейлист
struct Playlist {
    std::string name;
    std::vector<std::string> trackIds;
    int currentTrackIndex = 0;
    bool shuffle = false;
    bool loop = true;
};

// Система музыки
class MusicSystem {
public:
    MusicSystem();
    ~MusicSystem();
    
    void Update(float deltaTime);
    
    // Управление треками
    void RegisterTrack(const std::string& id, const std::string& filepath, bool loop = true);
    void RegisterTrackWithLayers(const std::string& id, const std::string& baseFilepath, 
                                  const std::vector<std::string>& layerFilepaths);
    
    void Play(const std::string& trackId, float fadeInTime = 0.0f);
    void Stop(const std::string& trackId, float fadeOutTime = 0.0f);
    void Pause(const std::string& trackId);
    void Resume(const std::string& trackId);
    
    // Кроссфейд между треками
    void Crossfade(const std::string& fromTrackId, const std::string& toTrackId, float duration);
    void CrossfadeToTrack(const std::string& trackId, float duration);  // Из текущего трека
    
    // Громкость
    void SetTrackVolume(const std::string& trackId, float volume);
    void SetMasterVolume(float volume) { m_MasterVolume = volume; }
    float GetMasterVolume() const { return m_MasterVolume; }
    
    // Адаптивная музыка - управление слоями
    void SetLayerActive(const std::string& trackId, const std::string& layerId, bool active, float fadeTime = 1.0f);
    void SetLayerVolume(const std::string& trackId, const std::string& layerId, float volume);
    void FadeInLayer(const std::string& trackId, const std::string& layerId, float duration);
    void FadeOutLayer(const std::string& trackId, const std::string& layerId, float duration);
    
    // Плейлисты
    void CreatePlaylist(const std::string& name, const std::vector<std::string>& trackIds, bool shuffle = false, bool loop = true);
    void PlayPlaylist(const std::string& name, float crossfadeDuration = 2.0f);
    void NextTrack(float crossfadeDuration = 2.0f);
    void PreviousTrack(float crossfadeDuration = 2.0f);
    
    // Состояние
    MusicState GetTrackState(const std::string& trackId) const;
    std::string GetCurrentTrack() const { return m_CurrentTrackId; }
    bool IsPlaying(const std::string& trackId) const;
    
    // Callbacks
    void SetOnTrackStart(std::function<void(const std::string&)> callback) {
        m_OnTrackStart = callback;
    }
    
    void SetOnTrackEnd(std::function<void(const std::string&)> callback) {
        m_OnTrackEnd = callback;
    }
    
    void SetOnCrossfadeComplete(std::function<void(const std::string&, const std::string&)> callback) {
        m_OnCrossfadeComplete = callback;
    }
    
private:
    std::unordered_map<std::string, MusicTrack> m_Tracks;
    std::unordered_map<std::string, MusicLayer> m_Layers;
    std::unordered_map<std::string, Playlist> m_Playlists;
    
    std::string m_CurrentTrackId;
    std::string m_CurrentPlaylist;
    
    float m_MasterVolume = 1.0f;
    
    // Кроссфейд
    bool m_IsCrossfading = false;
    std::string m_CrossfadeFromTrack;
    std::string m_CrossfadeToTrack;
    float m_CrossfadeProgress = 0.0f;
    float m_CrossfadeDuration = 2.0f;
    
    // Callbacks
    std::function<void(const std::string&)> m_OnTrackStart;
    std::function<void(const std::string&)> m_OnTrackEnd;
    std::function<void(const std::string&, const std::string&)> m_OnCrossfadeComplete;
    
    // Внутренние функции
    void UpdateTrack(MusicTrack& track, float deltaTime);
    void UpdateLayer(MusicLayer& layer, float deltaTime);
    void UpdateCrossfade(float deltaTime);
    void UpdatePlaylist();
    
    void StartTrack(const std::string& trackId, float fadeInTime);
    void StopTrack(const std::string& trackId, float fadeOutTime);
};

// Система вариаций звуков
class SoundVariationSystem {
public:
    struct SoundVariation {
        std::string id;
        std::vector<std::string> soundFiles;  // Пул звуков
        
        float minPitch = 0.9f;
        float maxPitch = 1.1f;
        
        float minVolume = 0.9f;
        float maxVolume = 1.0f;
        
        bool randomPitch = true;
        bool randomVolume = true;
        bool randomSelection = true;  // Случайный выбор из пула
        
        int lastPlayedIndex = -1;  // Для избежания повторов
    };
    
    SoundVariationSystem();
    
    // Регистрация вариаций
    void RegisterVariation(const std::string& id, const std::vector<std::string>& soundFiles);
    void SetPitchRange(const std::string& id, float minPitch, float maxPitch);
    void SetVolumeRange(const std::string& id, float minVolume, float maxVolume);
    void SetRandomization(const std::string& id, bool randomPitch, bool randomVolume, bool randomSelection);
    
    // Воспроизведение с вариациями
    struct PlaybackParams {
        std::string soundFile;
        float pitch = 1.0f;
        float volume = 1.0f;
    };
    
    PlaybackParams GetVariation(const std::string& id);
    void PlayVariation(const std::string& id);  // Использует внутренний аудио-движок
    
    // Callback для фактического воспроизведения
    void SetPlayCallback(std::function<void(const std::string&, float, float)> callback) {
        m_PlayCallback = callback;
    }
    
private:
    std::unordered_map<std::string, SoundVariation> m_Variations;
    std::function<void(const std::string& file, float pitch, float volume)> m_PlayCallback;
    
    float RandomRange(float min, float max);
    int GetNextSoundIndex(SoundVariation& variation);
};

} // namespace SAGE
