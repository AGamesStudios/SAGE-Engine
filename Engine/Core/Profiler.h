#pragma once

#include "../Core/Core.h"
#include "../Core/Logger.h"

#include <string>
#include <chrono>
#include <unordered_map>

namespace SAGE {

    // Таймер для измерения времени выполнения
    class ScopedTimer {
    public:
        ScopedTimer(const std::string& name);
        ~ScopedTimer();

    private:
        std::string m_Name;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
    };

    // Профилировщик производительности
    class Profiler {
    public:
        // Инициализация
        static void Init();
        static void Shutdown();

        // Обновление (вызывать каждый кадр)
        static void BeginFrame();
        static void EndFrame();

        // FPS и время кадра
        static float GetFPS();
        static float GetFrameTime(); // в миллисекундах
        static float GetDeltaTime(); // в секундах

        // Метрики рендеринга
        static void SetDrawCalls(size_t count);
        static void SetVertexCount(size_t count);
        static void SetTriangleCount(size_t count);
        static size_t GetDrawCalls();
        static size_t GetVertexCount();
        static size_t GetTriangleCount();

        // Память
        static void SetMemoryUsage(size_t bytes);
        static size_t GetMemoryUsage();
        static std::string GetMemoryUsageString(); // "12.5 MB"

        // GPU метрики (требует OpenGL context)
        static void EnableGPUProfiling(bool enable);
        static bool IsGPUProfilingEnabled();
        static void BeginGPUFrame();
        static void EndGPUFrame();
        static float GetGPUTime(); // в миллисекундах
        static size_t GetGPUMemoryUsed(); // в байтах (если поддерживается)

        // Пользовательские метрики
        static void RecordMetric(const std::string& name, float value);
        static float GetMetric(const std::string& name);

        // Таймеры
        static void BeginTimer(const std::string& name);
        static void EndTimer(const std::string& name);
        static float GetTimerDuration(const std::string& name); // в миллисекундах

        // Вывод статистики
        static void PrintStats();
        static void PrintTimers();

        // Настройки
        static void SetTargetFPS(float fps);
        static float GetTargetFPS();

    private:
        Profiler() = default;

        struct TimerData {
            std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
            float duration = 0.0f; // в миллисекундах
            bool isRunning = false;
        };

        static bool s_Initialized;
        
        // FPS
        static std::chrono::time_point<std::chrono::high_resolution_clock> s_FrameStart;
        static std::chrono::time_point<std::chrono::high_resolution_clock> s_LastFrameTime;
        static float s_DeltaTime;
        static float s_FrameTime;
        static float s_FPS;
        static float s_TargetFPS;
        
        // Счётчики кадров для усреднения FPS
        static size_t s_FrameCount;
        static float s_FPSAccumulator;
        static std::chrono::time_point<std::chrono::high_resolution_clock> s_FPSUpdateTime;

        // Метрики рендеринга
        static size_t s_DrawCalls;
        static size_t s_VertexCount;
        static size_t s_TriangleCount;

        // Память
        static size_t s_MemoryUsage;

        // Пользовательские метрики
        static std::unordered_map<std::string, float> s_Metrics;

        // Таймеры
        static std::unordered_map<std::string, TimerData> s_Timers;

        // GPU профилирование
        static bool s_GPUProfilingEnabled;
        static unsigned int s_GPUQueryBegin;
        static unsigned int s_GPUQueryEnd;
        static float s_GPUTime;
        static size_t s_GPUMemoryUsed;
    };

    // Макрос для автоматического профилирования области кода
    #define SAGE_PROFILE_SCOPE(name) SAGE::ScopedTimer timer##__LINE__(name)
    #define SAGE_PROFILE_FUNCTION() SAGE_PROFILE_SCOPE(__FUNCTION__)

} // namespace SAGE
