#pragma once

#include <string>

struct ma_sound;

namespace SAGE {

    class Sound {
    public:
        Sound(const std::string& path, bool streaming = false);
        ~Sound();

        void Play(bool restart = true);
        void Stop();
        void SetLooping(bool loop);
        bool IsPlaying() const;

        const std::string& GetPath() const { return m_Path; }
        bool IsValid() const { return m_Initialized; }

    private:
        std::string m_Path;
        bool m_Streaming = false;
        bool m_Initialized = false;
        ma_sound* m_Sound = nullptr;
    };

} // namespace SAGE
