#include "MusicSystem.h"
#include <algorithm>
#include <random>
#include <cmath>

namespace SAGE {

// MusicSystem implementations
MusicSystem::MusicSystem() {
}

MusicSystem::~MusicSystem() {
}

void MusicSystem::Update(float deltaTime) {
    // Обновить активные треки
    for (auto& pair : m_Tracks) {
        UpdateTrack(pair.second, deltaTime);
    }
    
    // Обновить слои
    for (auto& pair : m_Layers) {
        UpdateLayer(pair.second, deltaTime);
    }
    
    // Обновить кроссфейд
    if (m_IsCrossfading) {
        UpdateCrossfade(deltaTime);
    }
    
    // Обновить плейлист
    if (!m_CurrentPlaylist.empty()) {
        UpdatePlaylist();
    }
}

void MusicSystem::RegisterTrack(const std::string& id, const std::string& filepath, bool loop) {
    MusicTrack track;
    track.id = id;
    track.filepath = filepath;
    track.loop = loop;
    
    m_Tracks[id] = track;
}

void MusicSystem::RegisterTrackWithLayers(const std::string& id, const std::string& baseFilepath,
                                           const std::vector<std::string>& layerFilepaths) {
    MusicTrack track;
    track.id = id;
    track.filepath = baseFilepath;
    track.loop = true;
    
    // Создать слои
    for (size_t i = 0; i < layerFilepaths.size(); ++i) {
        std::string layerId = id + "_layer_" + std::to_string(i);
        
        MusicLayer layer;
        layer.id = layerId;
        layer.filepath = layerFilepaths[i];
        layer.active = false;
        layer.volume = 0.0f;
        layer.fadeVolume = 0.0f;
        
        m_Layers[layerId] = layer;
        track.layers.push_back(layerId);
        track.layerVolumes[layerId] = 0.0f;
    }
    
    m_Tracks[id] = track;
}

void MusicSystem::Play(const std::string& trackId, float fadeInTime) {
    auto it = m_Tracks.find(trackId);
    if (it == m_Tracks.end()) {
        return;
    }
    
    // Остановить текущий трек если есть
    if (!m_CurrentTrackId.empty() && m_CurrentTrackId != trackId) {
        Stop(m_CurrentTrackId, 0.0f);
    }
    
    StartTrack(trackId, fadeInTime);
}

void MusicSystem::Stop(const std::string& trackId, float fadeOutTime) {
    StopTrack(trackId, fadeOutTime);
    
    if (m_CurrentTrackId == trackId) {
        m_CurrentTrackId.clear();
    }
}

void MusicSystem::Pause(const std::string& trackId) {
    auto it = m_Tracks.find(trackId);
    if (it != m_Tracks.end()) {
        it->second.state = MusicState::Paused;
    }
}

void MusicSystem::Resume(const std::string& trackId) {
    auto it = m_Tracks.find(trackId);
    if (it != m_Tracks.end()) {
        if (it->second.state == MusicState::Paused) {
            it->second.state = MusicState::Playing;
        }
    }
}

void MusicSystem::Crossfade(const std::string& fromTrackId, const std::string& toTrackId, float duration) {
    auto fromIt = m_Tracks.find(fromTrackId);
    auto toIt = m_Tracks.find(toTrackId);
    
    if (fromIt == m_Tracks.end() || toIt == m_Tracks.end()) {
        return;
    }
    
    m_IsCrossfading = true;
    m_CrossfadeFromTrack = fromTrackId;
    m_CrossfadeToTrack = toTrackId;
    m_CrossfadeProgress = 0.0f;
    m_CrossfadeDuration = duration;
    
    // Начать новый трек с нулевой громкостью
    StartTrack(toTrackId, 0.0f);
    toIt->second.fadeVolume = 0.0f;
    toIt->second.volume = 0.0f;
}

void MusicSystem::CrossfadeToTrack(const std::string& trackId, float duration) {
    if (m_CurrentTrackId.empty()) {
        Play(trackId);
        return;
    }
    
    Crossfade(m_CurrentTrackId, trackId, duration);
}

void MusicSystem::SetTrackVolume(const std::string& trackId, float volume) {
    auto it = m_Tracks.find(trackId);
    if (it != m_Tracks.end()) {
        it->second.defaultVolume = std::clamp(volume, 0.0f, 1.0f);
    }
}

void MusicSystem::SetLayerActive(const std::string& trackId, const std::string& layerId, bool active, float fadeTime) {
    auto layerIt = m_Layers.find(layerId);
    if (layerIt == m_Layers.end()) {
        return;
    }
    
    MusicLayer& layer = layerIt->second;
    
    if (active) {
        FadeInLayer(trackId, layerId, fadeTime);
    } else {
        FadeOutLayer(trackId, layerId, fadeTime);
    }
}

void MusicSystem::SetLayerVolume(const std::string& trackId, const std::string& layerId, float volume) {
    auto trackIt = m_Tracks.find(trackId);
    auto layerIt = m_Layers.find(layerId);
    
    if (trackIt != m_Tracks.end() && layerIt != m_Layers.end()) {
        trackIt->second.layerVolumes[layerId] = std::clamp(volume, 0.0f, 1.0f);
        layerIt->second.volume = std::clamp(volume, 0.0f, 1.0f);
    }
}

void MusicSystem::FadeInLayer([[maybe_unused]] const std::string& trackId, const std::string& layerId, float duration) {
    auto layerIt = m_Layers.find(layerId);
    if (layerIt == m_Layers.end()) {
        return;
    }
    
    MusicLayer& layer = layerIt->second;
    layer.active = true;
    layer.targetVolume = 1.0f;
    layer.fadeSpeed = (duration > 0.0f) ? (1.0f / duration) : 999.0f;
}

void MusicSystem::FadeOutLayer([[maybe_unused]] const std::string& trackId, const std::string& layerId, float duration) {
    auto layerIt = m_Layers.find(layerId);
    if (layerIt == m_Layers.end()) {
        return;
    }
    
    MusicLayer& layer = layerIt->second;
    layer.targetVolume = 0.0f;
    layer.fadeSpeed = (duration > 0.0f) ? (1.0f / duration) : 999.0f;
}

void MusicSystem::CreatePlaylist(const std::string& name, const std::vector<std::string>& trackIds, 
                                  bool shuffle, bool loop) {
    Playlist playlist;
    playlist.name = name;
    playlist.trackIds = trackIds;
    playlist.shuffle = shuffle;
    playlist.loop = loop;
    playlist.currentTrackIndex = 0;
    
    if (shuffle && !trackIds.empty()) {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(playlist.trackIds.begin(), playlist.trackIds.end(), g);
    }
    
    m_Playlists[name] = playlist;
}

void MusicSystem::PlayPlaylist(const std::string& name, float crossfadeDuration) {
    auto it = m_Playlists.find(name);
    if (it == m_Playlists.end() || it->second.trackIds.empty()) {
        return;
    }
    
    m_CurrentPlaylist = name;
    Playlist& playlist = it->second;
    playlist.currentTrackIndex = 0;
    
    const std::string& firstTrack = playlist.trackIds[0];
    
    if (m_CurrentTrackId.empty()) {
        Play(firstTrack);
    } else {
        CrossfadeToTrack(firstTrack, crossfadeDuration);
    }
}

void MusicSystem::NextTrack(float crossfadeDuration) {
    if (m_CurrentPlaylist.empty()) {
        return;
    }
    
    auto it = m_Playlists.find(m_CurrentPlaylist);
    if (it == m_Playlists.end()) {
        return;
    }
    
    Playlist& playlist = it->second;
    playlist.currentTrackIndex++;
    
    if (playlist.currentTrackIndex >= static_cast<int>(playlist.trackIds.size())) {
        if (playlist.loop) {
            playlist.currentTrackIndex = 0;
        } else {
            m_CurrentPlaylist.clear();
            return;
        }
    }
    
    const std::string& nextTrack = playlist.trackIds[playlist.currentTrackIndex];
    CrossfadeToTrack(nextTrack, crossfadeDuration);
}

void MusicSystem::PreviousTrack(float crossfadeDuration) {
    if (m_CurrentPlaylist.empty()) {
        return;
    }
    
    auto it = m_Playlists.find(m_CurrentPlaylist);
    if (it == m_Playlists.end()) {
        return;
    }
    
    Playlist& playlist = it->second;
    playlist.currentTrackIndex--;
    
    if (playlist.currentTrackIndex < 0) {
        if (playlist.loop) {
            playlist.currentTrackIndex = static_cast<int>(playlist.trackIds.size()) - 1;
        } else {
            playlist.currentTrackIndex = 0;
        }
    }
    
    const std::string& prevTrack = playlist.trackIds[playlist.currentTrackIndex];
    CrossfadeToTrack(prevTrack, crossfadeDuration);
}

MusicState MusicSystem::GetTrackState(const std::string& trackId) const {
    auto it = m_Tracks.find(trackId);
    return (it != m_Tracks.end()) ? it->second.state : MusicState::Stopped;
}

bool MusicSystem::IsPlaying(const std::string& trackId) const {
    auto it = m_Tracks.find(trackId);
    return (it != m_Tracks.end() && it->second.state == MusicState::Playing);
}

void MusicSystem::UpdateTrack(MusicTrack& track, float deltaTime) {
    if (track.state == MusicState::Stopped || track.state == MusicState::Paused) {
        return;
    }
    
    // Обновить фейд
    if (track.state == MusicState::FadingIn) {
        track.fadeVolume += track.fadeSpeed * deltaTime;
        
        if (track.fadeVolume >= track.targetVolume) {
            track.fadeVolume = track.targetVolume;
            track.state = MusicState::Playing;
        }
        
        track.volume = track.fadeVolume * track.defaultVolume;
    }
    else if (track.state == MusicState::FadingOut) {
        track.fadeVolume -= track.fadeSpeed * deltaTime;
        
        if (track.fadeVolume <= 0.0f) {
            track.fadeVolume = 0.0f;
            track.state = MusicState::Stopped;
        }
        
        track.volume = track.fadeVolume * track.defaultVolume;
    }
    
    // Обновить время воспроизведения
    if (track.state == MusicState::Playing || track.state == MusicState::FadingIn || 
        track.state == MusicState::FadingOut) {
        track.currentTime += deltaTime;
        
        // Проверить конец трека
        if (track.duration > 0.0f && track.currentTime >= track.duration) {
            if (track.loop) {
                track.currentTime = 0.0f;
            } else {
                track.state = MusicState::Stopped;
                
                if (m_OnTrackEnd) {
                    m_OnTrackEnd(track.id);
                }
            }
        }
    }
}

void MusicSystem::UpdateLayer(MusicLayer& layer, float deltaTime) {
    if (!layer.active && layer.fadeVolume <= 0.0f) {
        return;
    }
    
    // Фейд слоя
    if (layer.fadeVolume < layer.targetVolume) {
        layer.fadeVolume += layer.fadeSpeed * deltaTime;
        
        if (layer.fadeVolume >= layer.targetVolume) {
            layer.fadeVolume = layer.targetVolume;
        }
    }
    else if (layer.fadeVolume > layer.targetVolume) {
        layer.fadeVolume -= layer.fadeSpeed * deltaTime;
        
        if (layer.fadeVolume <= layer.targetVolume) {
            layer.fadeVolume = layer.targetVolume;
            
            if (layer.fadeVolume <= 0.0f) {
                layer.active = false;
            }
        }
    }
    
    layer.volume = layer.fadeVolume;
}

void MusicSystem::UpdateCrossfade(float deltaTime) {
    m_CrossfadeProgress += deltaTime;
    
    float t = std::clamp(m_CrossfadeProgress / m_CrossfadeDuration, 0.0f, 1.0f);
    
    auto fromIt = m_Tracks.find(m_CrossfadeFromTrack);
    auto toIt = m_Tracks.find(m_CrossfadeToTrack);
    
    if (fromIt != m_Tracks.end()) {
        fromIt->second.volume = (1.0f - t) * fromIt->second.defaultVolume;
    }
    
    if (toIt != m_Tracks.end()) {
        toIt->second.volume = t * toIt->second.defaultVolume;
    }
    
    if (m_CrossfadeProgress >= m_CrossfadeDuration) {
        // Завершить кроссфейд
        if (fromIt != m_Tracks.end()) {
            fromIt->second.state = MusicState::Stopped;
            fromIt->second.volume = 0.0f;
        }
        
        if (toIt != m_Tracks.end()) {
            toIt->second.state = MusicState::Playing;
        }
        
        m_CurrentTrackId = m_CrossfadeToTrack;
        m_IsCrossfading = false;
        
        if (m_OnCrossfadeComplete) {
            m_OnCrossfadeComplete(m_CrossfadeFromTrack, m_CrossfadeToTrack);
        }
    }
}

void MusicSystem::UpdatePlaylist() {
    if (m_CurrentPlaylist.empty() || m_CurrentTrackId.empty()) {
        return;
    }
    
    // Проверить, закончился ли текущий трек
    auto trackIt = m_Tracks.find(m_CurrentTrackId);
    if (trackIt != m_Tracks.end() && trackIt->second.state == MusicState::Stopped) {
        NextTrack(2.0f);
    }
}

void MusicSystem::StartTrack(const std::string& trackId, float fadeInTime) {
    auto it = m_Tracks.find(trackId);
    if (it == m_Tracks.end()) {
        return;
    }
    
    MusicTrack& track = it->second;
    
    if (fadeInTime > 0.0f) {
        track.state = MusicState::FadingIn;
        track.fadeVolume = 0.0f;
        track.targetVolume = 1.0f;
        track.fadeSpeed = 1.0f / fadeInTime;
    } else {
        track.state = MusicState::Playing;
        track.fadeVolume = 1.0f;
        track.volume = track.defaultVolume;
    }
    
    track.currentTime = 0.0f;
    m_CurrentTrackId = trackId;
    
    if (m_OnTrackStart) {
        m_OnTrackStart(trackId);
    }
}

void MusicSystem::StopTrack(const std::string& trackId, float fadeOutTime) {
    auto it = m_Tracks.find(trackId);
    if (it == m_Tracks.end()) {
        return;
    }
    
    MusicTrack& track = it->second;
    
    if (fadeOutTime > 0.0f) {
        track.state = MusicState::FadingOut;
        track.fadeSpeed = 1.0f / fadeOutTime;
    } else {
        track.state = MusicState::Stopped;
        track.volume = 0.0f;
        track.fadeVolume = 0.0f;
    }
}

// SoundVariationSystem implementations
SoundVariationSystem::SoundVariationSystem() {
}

void SoundVariationSystem::RegisterVariation(const std::string& id, const std::vector<std::string>& soundFiles) {
    SoundVariation variation;
    variation.id = id;
    variation.soundFiles = soundFiles;
    
    m_Variations[id] = variation;
}

void SoundVariationSystem::SetPitchRange(const std::string& id, float minPitch, float maxPitch) {
    auto it = m_Variations.find(id);
    if (it != m_Variations.end()) {
        it->second.minPitch = minPitch;
        it->second.maxPitch = maxPitch;
    }
}

void SoundVariationSystem::SetVolumeRange(const std::string& id, float minVolume, float maxVolume) {
    auto it = m_Variations.find(id);
    if (it != m_Variations.end()) {
        it->second.minVolume = minVolume;
        it->second.maxVolume = maxVolume;
    }
}

void SoundVariationSystem::SetRandomization(const std::string& id, bool randomPitch, bool randomVolume, bool randomSelection) {
    auto it = m_Variations.find(id);
    if (it != m_Variations.end()) {
        it->second.randomPitch = randomPitch;
        it->second.randomVolume = randomVolume;
        it->second.randomSelection = randomSelection;
    }
}

SoundVariationSystem::PlaybackParams SoundVariationSystem::GetVariation(const std::string& id) {
    PlaybackParams params;
    
    auto it = m_Variations.find(id);
    if (it == m_Variations.end() || it->second.soundFiles.empty()) {
        return params;
    }
    
    SoundVariation& variation = it->second;
    
    // Выбрать файл
    int soundIndex = GetNextSoundIndex(variation);
    params.soundFile = variation.soundFiles[soundIndex];
    
    // Случайный pitch
    if (variation.randomPitch) {
        params.pitch = RandomRange(variation.minPitch, variation.maxPitch);
    }
    
    // Случайная громкость
    if (variation.randomVolume) {
        params.volume = RandomRange(variation.minVolume, variation.maxVolume);
    }
    
    return params;
}

void SoundVariationSystem::PlayVariation(const std::string& id) {
    PlaybackParams params = GetVariation(id);
    
    if (!params.soundFile.empty() && m_PlayCallback) {
        m_PlayCallback(params.soundFile, params.pitch, params.volume);
    }
}

float SoundVariationSystem::RandomRange(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

int SoundVariationSystem::GetNextSoundIndex(SoundVariation& variation) {
    if (variation.soundFiles.empty()) {
        return 0;
    }
    
    if (!variation.randomSelection) {
        // Последовательный выбор
        variation.lastPlayedIndex = (variation.lastPlayedIndex + 1) % static_cast<int>(variation.soundFiles.size());
        return variation.lastPlayedIndex;
    }
    
    // Случайный выбор (но избегать повторения последнего)
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, static_cast<int>(variation.soundFiles.size()) - 1);
    
    int index = dis(gen);
    
    // Избежать повтора, если звуков больше одного
    if (variation.soundFiles.size() > 1 && index == variation.lastPlayedIndex) {
        index = (index + 1) % static_cast<int>(variation.soundFiles.size());
    }
    
    variation.lastPlayedIndex = index;
    return index;
}

} // namespace SAGE
