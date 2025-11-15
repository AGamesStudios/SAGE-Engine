#include "../TestFramework.h"
#include "Audio/AudioSystem.h"
#include <thread>
#include <chrono>

using namespace SAGE;

TEST(AudioSystem_Initialization) {
    AudioSystem audio;
    ASSERT(!audio.IsInitialized());
    
    bool result = audio.Init();
    ASSERT(result);
    ASSERT(audio.IsInitialized());
    
    audio.Shutdown();
    ASSERT(!audio.IsInitialized());
}

TEST(AudioSystem_MasterVolume) {
    AudioSystem audio;
    audio.Init();
    
    // Default master volume
    float defaultVol = audio.GetMasterVolume();
    ASSERT(defaultVol >= 0.0f && defaultVol <= 1.0f);
    
    // Set master volume
    audio.SetMasterVolume(0.5f);
    ASSERT_NEAR(audio.GetMasterVolume(), 0.5f, 0.01f);
    
    audio.SetMasterVolume(0.0f);
    ASSERT_NEAR(audio.GetMasterVolume(), 0.0f, 0.01f);
    
    audio.SetMasterVolume(1.0f);
    ASSERT_NEAR(audio.GetMasterVolume(), 1.0f, 0.01f);
    
    audio.Shutdown();
}

TEST(AudioSystem_CategoryVolume) {
    AudioSystem audio;
    audio.Init();
    
    // Set SFX volume
    audio.SetSFXVolume(0.7f);
    ASSERT_NEAR(audio.GetSFXVolume(), 0.7f, 0.01f);
    
    // Set BGM volume
    audio.SetBGMVolume(0.3f);
    ASSERT_NEAR(audio.GetBGMVolume(), 0.3f, 0.01f);
    
    // Set category volumes
    audio.SetCategoryVolume(AudioCategory::SFX, 0.8f);
    ASSERT_NEAR(audio.GetCategoryVolume(AudioCategory::SFX), 0.8f, 0.01f);
    
    audio.SetCategoryVolume(AudioCategory::Music, 0.4f);
    ASSERT_NEAR(audio.GetCategoryVolume(AudioCategory::Music), 0.4f, 0.01f);
    
    audio.Shutdown();
}

TEST(AudioSystem_ListenerPosition) {
    AudioSystem audio;
    audio.Init();
    
    // Set listener position (should not crash)
    audio.SetListenerPosition(100.0f, 200.0f, 0.0f);
    audio.SetListenerPosition(-50.0f, 75.5f, 10.0f);
    
    // Set listener velocity
    audio.SetListenerVelocity(5.0f, -2.0f, 0.0f);
    
    audio.Shutdown();
}

TEST(AudioSystem_StopAll) {
    AudioSystem audio;
    audio.Init();
    
    // StopAll should work even without any sounds
    audio.StopAll();
    audio.StopAllSFX();
    
    audio.Shutdown();
}

TEST(AudioSystem_PauseResume) {
    AudioSystem audio;
    audio.Init();
    
    // Pause/Resume should work even without active sounds
    audio.PauseAll();
    audio.ResumeAll();
    audio.PauseBGM();
    audio.ResumeBGM();
    
    ASSERT(!audio.IsBGMPlaying());
    
    audio.Shutdown();
}

TEST(AudioSystem_Update) {
    AudioSystem audio;
    audio.Init();
    
    // Update should work without crashing
    for (int i = 0; i < 10; ++i) {
        audio.Update(0.016f); // 60 FPS
    }
    
    audio.Shutdown();
}

TEST(AudioSystem_StressTest_Initialization) {
    // Stress test: multiple init/shutdown cycles
    AudioSystem audio;
    
    for (int i = 0; i < 5; ++i) {
        bool result = audio.Init();
        ASSERT(result);
        ASSERT(audio.IsInitialized());
        
        audio.Shutdown();
        ASSERT(!audio.IsInitialized());
    }
}

TEST(AudioSystem_VolumeRange) {
    AudioSystem audio;
    audio.Init();
    
    // Test volume clamping
    audio.SetMasterVolume(-1.0f);  // Should clamp to 0
    float vol = audio.GetMasterVolume();
    ASSERT(vol >= 0.0f);
    
    audio.SetMasterVolume(2.0f);   // Should clamp to 1
    vol = audio.GetMasterVolume();
    ASSERT(vol <= 1.0f);
    
    audio.Shutdown();
}
