#include "Sound.h"

#include "AudioSystem.h"
#include "../Core/Logger.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4505 4456 4245)
#endif
#include <miniaudio.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace SAGE {

    Sound::Sound(const std::string& path, bool streaming)
        : m_Path(path), m_Streaming(streaming) {
        if (!AudioSystem::IsInitialized()) {
            SAGE_WARNING("AudioSystem не был инициализирован. Автоматический запуск.");
            AudioSystem::Init();
        }

        ma_engine* engine = AudioSystem::GetEngine();
        if (!engine) {
            SAGE_ERROR("ma_engine недоступен. Звук '{}' не будет загружен.", path);
            return;
        }

        m_Sound = new ma_sound();
        ma_uint32 flags = m_Streaming ? MA_SOUND_FLAG_STREAM : 0;
        ma_result result = ma_sound_init_from_file(engine, path.c_str(), flags, nullptr, nullptr, m_Sound);
        if (result != MA_SUCCESS) {
            SAGE_ERROR("Не удалось загрузить звук '{}', код: {}", path, result);
            delete m_Sound;
            m_Sound = nullptr;
            return;
        }

        m_Initialized = true;
    }

    Sound::~Sound() {
        if (m_Sound) {
            ma_sound_uninit(m_Sound);
            delete m_Sound;
            m_Sound = nullptr;
        }
    }

    void Sound::Play(bool restart) {
        if (!m_Initialized || !m_Sound)
            return;

        if (restart)
            ma_sound_seek_to_pcm_frame(m_Sound, 0);

        ma_result result = ma_sound_start(m_Sound);
        if (result != MA_SUCCESS) {
            SAGE_ERROR("Не удалось воспроизвести звук '{}', код: {}", m_Path, result);
        }
    }

    void Sound::Stop() {
        if (!m_Initialized || !m_Sound)
            return;

        ma_sound_stop(m_Sound);
    }

    void Sound::SetLooping(bool loop) {
        if (!m_Initialized || !m_Sound)
            return;

        ma_sound_set_looping(m_Sound, loop ? MA_TRUE : MA_FALSE);
    }

    bool Sound::IsPlaying() const {
        if (!m_Initialized || !m_Sound)
            return false;

        return ma_sound_is_playing(m_Sound) == MA_TRUE;
    }

} // namespace SAGE
