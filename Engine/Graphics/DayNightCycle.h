#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>

namespace SAGE {

using json = nlohmann::json;

// Время суток
enum class TimeOfDay {
    Night,      // 0:00 - 6:00
    Dawn,       // 6:00 - 8:00
    Morning,    // 8:00 - 12:00
    Noon,       // 12:00 - 14:00
    Afternoon,  // 14:00 - 18:00
    Dusk,       // 18:00 - 20:00
    Evening     // 20:00 - 24:00
};

// Погода
enum class WeatherType {
    Clear,
    Cloudy,
    Rainy,
    Stormy,
    Foggy,
    Snowy
};

// Информация о погоде
struct Weather {
    WeatherType type = WeatherType::Clear;
    float intensity = 0.0f;  // 0.0 - 1.0
    float windSpeed = 0.0f;
    glm::vec2 windDirection = glm::vec2(1.0f, 0.0f);
    
    float transitionTime = 0.0f;
    float transitionDuration = 5.0f;  // Секунды
    WeatherType targetWeather = WeatherType::Clear;
    bool transitioning = false;
    
    json ToJson() const;
    void FromJson(const json& j);
};

// Настройки освещения для времени суток
struct LightingSettings {
    glm::vec3 ambientColor = glm::vec3(1.0f);
    glm::vec3 directionalLightColor = glm::vec3(1.0f);
    glm::vec3 directionalLightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
    
    float ambientIntensity = 0.3f;
    float directionalIntensity = 0.7f;
    
    glm::vec3 skyColor = glm::vec3(0.5f, 0.7f, 1.0f);
    glm::vec3 horizonColor = glm::vec3(1.0f, 0.8f, 0.6f);
    
    float fogDensity = 0.0f;
    glm::vec3 fogColor = glm::vec3(0.5f);
    
    json ToJson() const;
    void FromJson(const json& j);
};

// Система день/ночь
class DayNightCycle {
public:
    DayNightCycle();
    
    // Обновление
    void Update(float deltaTime);
    
    // Время
    void SetTime(float hours);  // 0.0 - 24.0
    float GetTime() const { return m_CurrentTime; }
    
    void SetTimeScale(float scale) { m_TimeScale = scale; }
    float GetTimeScale() const { return m_TimeScale; }
    
    void SetDayDuration(float seconds) { m_DayDuration = seconds; }
    float GetDayDuration() const { return m_DayDuration; }
    
    TimeOfDay GetTimeOfDay() const;
    std::string GetTimeOfDayString() const;
    
    int GetHour() const { return static_cast<int>(m_CurrentTime); }
    int GetMinute() const { return static_cast<int>((m_CurrentTime - GetHour()) * 60.0f); }
    
    // Освещение
    LightingSettings GetCurrentLighting() const;
    
    void SetNightLighting(const LightingSettings& settings) { m_NightLighting = settings; }
    void SetDawnLighting(const LightingSettings& settings) { m_DawnLighting = settings; }
    void SetDayLighting(const LightingSettings& settings) { m_DayLighting = settings; }
    void SetDuskLighting(const LightingSettings& settings) { m_DuskLighting = settings; }
    
    const LightingSettings& GetNightLighting() const { return m_NightLighting; }
    const LightingSettings& GetDawnLighting() const { return m_DawnLighting; }
    const LightingSettings& GetDayLighting() const { return m_DayLighting; }
    const LightingSettings& GetDuskLighting() const { return m_DuskLighting; }
    
    // Погода
    void SetWeather(WeatherType type, float intensity = 1.0f);
    void TransitionWeather(WeatherType targetWeather, float duration);
    
    Weather& GetWeather() { return m_Weather; }
    const Weather& GetWeather() const { return m_Weather; }
    
    WeatherType GetWeatherType() const { return m_Weather.type; }
    float GetWeatherIntensity() const { return m_Weather.intensity; }
    
    // Callbacks
    void SetOnTimeOfDayChange(std::function<void(TimeOfDay)> callback) {
        m_OnTimeOfDayChange = callback;
    }
    
    void SetOnHourChange(std::function<void(int hour)> callback) {
        m_OnHourChange = callback;
    }
    
    void SetOnWeatherChange(std::function<void(WeatherType)> callback) {
        m_OnWeatherChange = callback;
    }
    
    // Пауза
    void Pause() { m_Paused = true; }
    void Resume() { m_Paused = false; }
    void TogglePause() { m_Paused = !m_Paused; }
    bool IsPaused() const { return m_Paused; }
    
    // Сохранение/загрузка
    json ToJson() const;
    void FromJson(const json& j);
    
private:
    // Время
    float m_CurrentTime = 12.0f;  // 0.0 - 24.0 (часы)
    float m_TimeScale = 1.0f;     // Множитель скорости времени
    float m_DayDuration = 600.0f; // Продолжительность дня в секундах (10 минут по умолчанию)
    
    TimeOfDay m_CurrentTimeOfDay = TimeOfDay::Noon;
    int m_LastHour = 12;
    
    bool m_Paused = false;
    
    // Освещение
    LightingSettings m_NightLighting;
    LightingSettings m_DawnLighting;
    LightingSettings m_DayLighting;
    LightingSettings m_DuskLighting;
    
    // Погода
    Weather m_Weather;
    
    // Callbacks
    std::function<void(TimeOfDay)> m_OnTimeOfDayChange;
    std::function<void(int hour)> m_OnHourChange;
    std::function<void(WeatherType)> m_OnWeatherChange;
    
    // Вспомогательные функции
    void UpdateTimeOfDay();
    void UpdateWeather(float deltaTime);
    LightingSettings InterpolateLighting(const LightingSettings& a, const LightingSettings& b, float t) const;
    void InitializeDefaultLighting();
};

} // namespace SAGE
