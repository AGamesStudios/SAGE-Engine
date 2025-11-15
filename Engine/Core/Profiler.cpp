#include "Profiler.h"
#include <sstream>
#include <iomanip>
#include <glad/glad.h>

namespace SAGE {

    // Статические члены
    bool Profiler::s_Initialized = false;
    std::chrono::time_point<std::chrono::high_resolution_clock> Profiler::s_FrameStart;
    std::chrono::time_point<std::chrono::high_resolution_clock> Profiler::s_LastFrameTime;
    float Profiler::s_DeltaTime = 0.0f;
    float Profiler::s_FrameTime = 0.0f;
    float Profiler::s_FPS = 0.0f;
    float Profiler::s_TargetFPS = 60.0f;
    size_t Profiler::s_FrameCount = 0;
    float Profiler::s_FPSAccumulator = 0.0f;
    std::chrono::time_point<std::chrono::high_resolution_clock> Profiler::s_FPSUpdateTime;

    size_t Profiler::s_DrawCalls = 0;
    size_t Profiler::s_VertexCount = 0;
    size_t Profiler::s_TriangleCount = 0;
    size_t Profiler::s_MemoryUsage = 0;

    std::unordered_map<std::string, float> Profiler::s_Metrics;
    std::unordered_map<std::string, Profiler::TimerData> Profiler::s_Timers;

    bool Profiler::s_GPUProfilingEnabled = false;
    unsigned int Profiler::s_GPUQueryBegin = 0;
    unsigned int Profiler::s_GPUQueryEnd = 0;
    float Profiler::s_GPUTime = 0.0f;
    size_t Profiler::s_GPUMemoryUsed = 0;

    // ============ ScopedTimer ============

    ScopedTimer::ScopedTimer(const std::string& name)
        : m_Name(name)
        , m_StartTime(std::chrono::high_resolution_clock::now()) {
    }

    ScopedTimer::~ScopedTimer() {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_StartTime).count();
        float durationMs = duration / 1000.0f;

        // Записываем результат в профайлер
        Profiler::RecordMetric(m_Name, durationMs);
    }

    // ============ Profiler ============

    void Profiler::Init() {
        if (s_Initialized) {
            SAGE_WARNING("Profiler уже инициализирован");
            return;
        }

        s_FrameStart = std::chrono::high_resolution_clock::now();
        s_LastFrameTime = s_FrameStart;
        s_FPSUpdateTime = s_FrameStart;

        // Note: GPU profiling requires OpenGL 3.3+ with ARB_timer_query extension
        // This will be added when GLAD is updated to support GL 3.3+

        SAGE_INFO("Profiler инициализирован");
        s_Initialized = true;
    }

    void Profiler::Shutdown() {
        if (!s_Initialized) {
            return;
        }

        // Note: GPU query cleanup will be added with GPU profiling support

        SAGE_INFO("Profiler завершён");
        s_Metrics.clear();
        s_Timers.clear();
        s_Initialized = false;
    }

    void Profiler::BeginFrame() {
        s_FrameStart = std::chrono::high_resolution_clock::now();

        // Сброс счётчиков рендеринга
        s_DrawCalls = 0;
        s_VertexCount = 0;
        s_TriangleCount = 0;
    }

    void Profiler::EndFrame() {
        auto frameEnd = std::chrono::high_resolution_clock::now();

        // Вычисление времени кадра
        auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - s_FrameStart).count();
        s_FrameTime = frameDuration / 1000.0f; // в миллисекундах

        // Вычисление delta time
        auto deltaTimeMicro = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - s_LastFrameTime).count();
        s_DeltaTime = deltaTimeMicro / 1000000.0f; // в секундах
        s_LastFrameTime = frameEnd;

        // Обновление FPS (усреднение за 0.5 секунды)
        s_FrameCount++;
        s_FPSAccumulator += s_DeltaTime;

        auto timeSinceUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - s_FPSUpdateTime).count();
        if (timeSinceUpdate >= 500) { // Обновляем каждые 500мс
            if (s_FPSAccumulator > 0.0f) {
                s_FPS = s_FrameCount / s_FPSAccumulator;
            }
            s_FrameCount = 0;
            s_FPSAccumulator = 0.0f;
            s_FPSUpdateTime = frameEnd;
        }
    }

    float Profiler::GetFPS() {
        return s_FPS;
    }

    float Profiler::GetFrameTime() {
        return s_FrameTime;
    }

    float Profiler::GetDeltaTime() {
        return s_DeltaTime;
    }

    void Profiler::SetTargetFPS(float fps) {
        s_TargetFPS = fps;
    }

    float Profiler::GetTargetFPS() {
        return s_TargetFPS;
    }

    // ============ Метрики рендеринга ============

    void Profiler::SetDrawCalls(size_t count) {
        s_DrawCalls = count;
    }

    void Profiler::SetVertexCount(size_t count) {
        s_VertexCount = count;
    }

    void Profiler::SetTriangleCount(size_t count) {
        s_TriangleCount = count;
    }

    size_t Profiler::GetDrawCalls() {
        return s_DrawCalls;
    }

    size_t Profiler::GetVertexCount() {
        return s_VertexCount;
    }

    size_t Profiler::GetTriangleCount() {
        return s_TriangleCount;
    }

    // ============ Память ============

    void Profiler::SetMemoryUsage(size_t bytes) {
        s_MemoryUsage = bytes;
    }

    size_t Profiler::GetMemoryUsage() {
        return s_MemoryUsage;
    }

    std::string Profiler::GetMemoryUsageString() {
        float mb = s_MemoryUsage / (1024.0f * 1024.0f);
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << mb << " MB";
        return oss.str();
    }

    // ============ Пользовательские метрики ============

    void Profiler::RecordMetric(const std::string& name, float value) {
        s_Metrics[name] = value;
    }

    float Profiler::GetMetric(const std::string& name) {
        auto it = s_Metrics.find(name);
        if (it != s_Metrics.end()) {
            return it->second;
        }
        return 0.0f;
    }

    // ============ Таймеры ============

    void Profiler::BeginTimer(const std::string& name) {
        auto& timer = s_Timers[name];
        timer.startTime = std::chrono::high_resolution_clock::now();
        timer.isRunning = true;
    }

    void Profiler::EndTimer(const std::string& name) {
        auto it = s_Timers.find(name);
        if (it == s_Timers.end() || !it->second.isRunning) {
            SAGE_WARNING("Таймер '{}' не был запущен", name);
            return;
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - it->second.startTime).count();
        it->second.duration = duration / 1000.0f; // в миллисекундах
        it->second.isRunning = false;
    }

    float Profiler::GetTimerDuration(const std::string& name) {
        auto it = s_Timers.find(name);
        if (it != s_Timers.end()) {
            return it->second.duration;
        }
        return 0.0f;
    }

    // ============ Вывод статистики ============

    void Profiler::EnableGPUProfiling(bool enable) {
        s_GPUProfilingEnabled = enable;
    }

    bool Profiler::IsGPUProfilingEnabled() {
        return s_GPUProfilingEnabled;
    }

    void Profiler::BeginGPUFrame() {
        // Note: GPU profiling will be implemented after GLAD upgrade to GL 3.3+
    }

    void Profiler::EndGPUFrame() {
        // Note: GPU profiling will be implemented after GLAD upgrade to GL 3.3+
    }

    float Profiler::GetGPUTime() {
        return s_GPUTime;
    }

    size_t Profiler::GetGPUMemoryUsed() {
        // Note: GPU memory query will be implemented after GLAD upgrade
        return 0;
    }

    // ============ Вывод статистики ============

    void Profiler::PrintStats() {
        SAGE_INFO("========== Profiler Statistics ==========");
        SAGE_INFO("FPS:         {:.1f} / {:.1f} (target)", s_FPS, s_TargetFPS);
        SAGE_INFO("Frame Time:  {:.2f} ms", s_FrameTime);
        SAGE_INFO("Delta Time:  {:.4f} s", s_DeltaTime);
        SAGE_INFO("");
        
        if (s_GPUProfilingEnabled) {
            SAGE_INFO("GPU Time:    {:.2f} ms", s_GPUTime);
            if (s_GPUMemoryUsed > 0) {
                SAGE_INFO("GPU Memory:  {:.2f} MB", s_GPUMemoryUsed / (1024.0f * 1024.0f));
            }
            SAGE_INFO("");
        }
        
        SAGE_INFO("Draw Calls:  {}", s_DrawCalls);
        SAGE_INFO("Vertices:    {}", s_VertexCount);
        SAGE_INFO("Triangles:   {}", s_TriangleCount);
        SAGE_INFO("");
        SAGE_INFO("Memory:      {}", GetMemoryUsageString());
        SAGE_INFO("=========================================");
    }

    void Profiler::PrintTimers() {
        if (s_Timers.empty()) {
            SAGE_INFO("Нет активных таймеров");
            return;
        }

        SAGE_INFO("========== Active Timers ==========");
        for (const auto& [name, timer] : s_Timers) {
            if (!timer.isRunning) {
                SAGE_INFO("{}: {:.3f} ms", name, timer.duration);
            } else {
                SAGE_INFO("{}: [running]", name);
            }
        }
        SAGE_INFO("===================================");
    }

} // namespace SAGE
