#include "DayNightCycle.h"
#include <algorithm>

namespace SAGE {

// Weather implementations
json Weather::ToJson() const {
    return json{
        {"type", static_cast<int>(type)},
        {"intensity", intensity},
        {"windSpeed", windSpeed},
        {"windDirection", {windDirection.x, windDirection.y}},
        {"transitionTime", transitionTime},
        {"transitionDuration", transitionDuration},
        {"targetWeather", static_cast<int>(targetWeather)},
        {"transitioning", transitioning}
    };
}

void Weather::FromJson(const json& j) {
    type = static_cast<WeatherType>(j.value("type", 0));
    intensity = j.value("intensity", 0.0f);
    windSpeed = j.value("windSpeed", 0.0f);
    
    if (j.contains("windDirection")) {
        auto dir = j["windDirection"];
        windDirection = glm::vec2(dir[0], dir[1]);
    }
    
    transitionTime = j.value("transitionTime", 0.0f);
    transitionDuration = j.value("transitionDuration", 5.0f);
    targetWeather = static_cast<WeatherType>(j.value("targetWeather", 0));
    transitioning = j.value("transitioning", false);
}

// LightingSettings implementations
json LightingSettings::ToJson() const {
    return json{
        {"ambientColor", {ambientColor.r, ambientColor.g, ambientColor.b}},
        {"directionalLightColor", {directionalLightColor.r, directionalLightColor.g, directionalLightColor.b}},
        {"directionalLightDirection", {directionalLightDirection.x, directionalLightDirection.y, directionalLightDirection.z}},
        {"ambientIntensity", ambientIntensity},
        {"directionalIntensity", directionalIntensity},
        {"skyColor", {skyColor.r, skyColor.g, skyColor.b}},
        {"horizonColor", {horizonColor.r, horizonColor.g, horizonColor.b}},
        {"fogDensity", fogDensity},
        {"fogColor", {fogColor.r, fogColor.g, fogColor.b}}
    };
}

void LightingSettings::FromJson(const json& j) {
    if (j.contains("ambientColor")) {
        auto ac = j["ambientColor"];
        ambientColor = glm::vec3(ac[0], ac[1], ac[2]);
    }
    
    if (j.contains("directionalLightColor")) {
        auto dlc = j["directionalLightColor"];
        directionalLightColor = glm::vec3(dlc[0], dlc[1], dlc[2]);
    }
    
    if (j.contains("directionalLightDirection")) {
        auto dld = j["directionalLightDirection"];
        directionalLightDirection = glm::vec3(dld[0], dld[1], dld[2]);
    }
    
    ambientIntensity = j.value("ambientIntensity", 0.3f);
    directionalIntensity = j.value("directionalIntensity", 0.7f);
    
    if (j.contains("skyColor")) {
        auto sc = j["skyColor"];
        skyColor = glm::vec3(sc[0], sc[1], sc[2]);
    }
    
    if (j.contains("horizonColor")) {
        auto hc = j["horizonColor"];
        horizonColor = glm::vec3(hc[0], hc[1], hc[2]);
    }
    
    fogDensity = j.value("fogDensity", 0.0f);
    
    if (j.contains("fogColor")) {
        auto fc = j["fogColor"];
        fogColor = glm::vec3(fc[0], fc[1], fc[2]);
    }
}

// DayNightCycle implementations
DayNightCycle::DayNightCycle() {
    InitializeDefaultLighting();
}

void DayNightCycle::Update(float deltaTime) {
    if (m_Paused) {
        return;
    }
    
    // Обновить время
    float timeIncrement = (deltaTime * m_TimeScale * 24.0f) / m_DayDuration;
    m_CurrentTime += timeIncrement;
    
    // Зациклить время
    if (m_CurrentTime >= 24.0f) {
        m_CurrentTime -= 24.0f;
    }
    
    // Проверить смену часа
    int currentHour = GetHour();
    if (currentHour != m_LastHour) {
        m_LastHour = currentHour;
        
        if (m_OnHourChange) {
            m_OnHourChange(currentHour);
        }
    }
    
    // Проверить смену времени суток
    UpdateTimeOfDay();
    
    // Обновить погоду
    UpdateWeather(deltaTime);
}

void DayNightCycle::SetTime(float hours) {
    m_CurrentTime = std::clamp(hours, 0.0f, 24.0f);
    m_LastHour = GetHour();
    UpdateTimeOfDay();
}

TimeOfDay DayNightCycle::GetTimeOfDay() const {
    float time = m_CurrentTime;
    
    if (time >= 0.0f && time < 6.0f) {
        return TimeOfDay::Night;
    } else if (time >= 6.0f && time < 8.0f) {
        return TimeOfDay::Dawn;
    } else if (time >= 8.0f && time < 12.0f) {
        return TimeOfDay::Morning;
    } else if (time >= 12.0f && time < 14.0f) {
        return TimeOfDay::Noon;
    } else if (time >= 14.0f && time < 18.0f) {
        return TimeOfDay::Afternoon;
    } else if (time >= 18.0f && time < 20.0f) {
        return TimeOfDay::Dusk;
    } else {
        return TimeOfDay::Evening;
    }
}

std::string DayNightCycle::GetTimeOfDayString() const {
    switch (GetTimeOfDay()) {
        case TimeOfDay::Night:     return "Night";
        case TimeOfDay::Dawn:      return "Dawn";
        case TimeOfDay::Morning:   return "Morning";
        case TimeOfDay::Noon:      return "Noon";
        case TimeOfDay::Afternoon: return "Afternoon";
        case TimeOfDay::Dusk:      return "Dusk";
        case TimeOfDay::Evening:   return "Evening";
        default:                   return "Unknown";
    }
}

LightingSettings DayNightCycle::GetCurrentLighting() const {
    float time = m_CurrentTime;
    
    // Ночь (0:00 - 6:00)
    if (time >= 0.0f && time < 6.0f) {
        return m_NightLighting;
    }
    // Рассвет (6:00 - 8:00)
    else if (time >= 6.0f && time < 8.0f) {
        float t = (time - 6.0f) / 2.0f;
        return InterpolateLighting(m_NightLighting, m_DawnLighting, t);
    }
    // Утро/День (8:00 - 18:00)
    else if (time >= 8.0f && time < 18.0f) {
        return m_DayLighting;
    }
    // Закат (18:00 - 20:00)
    else if (time >= 18.0f && time < 20.0f) {
        float t = (time - 18.0f) / 2.0f;
        return InterpolateLighting(m_DuskLighting, m_NightLighting, t);
    }
    // Вечер/Ночь (20:00 - 24:00)
    else {
        return m_NightLighting;
    }
}

void DayNightCycle::SetWeather(WeatherType type, float intensity) {
    if (m_Weather.type != type && m_OnWeatherChange) {
        m_OnWeatherChange(type);
    }
    
    m_Weather.type = type;
    m_Weather.intensity = std::clamp(intensity, 0.0f, 1.0f);
    m_Weather.transitioning = false;
}

void DayNightCycle::TransitionWeather(WeatherType targetWeather, float duration) {
    m_Weather.targetWeather = targetWeather;
    m_Weather.transitionDuration = duration;
    m_Weather.transitionTime = 0.0f;
    m_Weather.transitioning = true;
}

void DayNightCycle::UpdateTimeOfDay() {
    TimeOfDay newTimeOfDay = GetTimeOfDay();
    
    if (newTimeOfDay != m_CurrentTimeOfDay) {
        m_CurrentTimeOfDay = newTimeOfDay;
        
        if (m_OnTimeOfDayChange) {
            m_OnTimeOfDayChange(newTimeOfDay);
        }
    }
}

void DayNightCycle::UpdateWeather(float deltaTime) {
    if (!m_Weather.transitioning) {
        return;
    }
    
    m_Weather.transitionTime += deltaTime;
    
    if (m_Weather.transitionTime >= m_Weather.transitionDuration) {
        // Завершить переход
        SetWeather(m_Weather.targetWeather, m_Weather.intensity);
    }
}

LightingSettings DayNightCycle::InterpolateLighting(const LightingSettings& a, const LightingSettings& b, float t) const {
    t = std::clamp(t, 0.0f, 1.0f);
    
    LightingSettings result;
    
    result.ambientColor = glm::mix(a.ambientColor, b.ambientColor, t);
    result.directionalLightColor = glm::mix(a.directionalLightColor, b.directionalLightColor, t);
    result.directionalLightDirection = glm::normalize(glm::mix(a.directionalLightDirection, b.directionalLightDirection, t));
    
    result.ambientIntensity = glm::mix(a.ambientIntensity, b.ambientIntensity, t);
    result.directionalIntensity = glm::mix(a.directionalIntensity, b.directionalIntensity, t);
    
    result.skyColor = glm::mix(a.skyColor, b.skyColor, t);
    result.horizonColor = glm::mix(a.horizonColor, b.horizonColor, t);
    
    result.fogDensity = glm::mix(a.fogDensity, b.fogDensity, t);
    result.fogColor = glm::mix(a.fogColor, b.fogColor, t);
    
    return result;
}

void DayNightCycle::InitializeDefaultLighting() {
    // Ночь - темно-синий, слабое освещение
    m_NightLighting.ambientColor = glm::vec3(0.1f, 0.15f, 0.3f);
    m_NightLighting.directionalLightColor = glm::vec3(0.2f, 0.25f, 0.4f);
    m_NightLighting.directionalLightDirection = glm::vec3(0.0f, -1.0f, -0.3f);
    m_NightLighting.ambientIntensity = 0.2f;
    m_NightLighting.directionalIntensity = 0.3f;
    m_NightLighting.skyColor = glm::vec3(0.05f, 0.05f, 0.15f);
    m_NightLighting.horizonColor = glm::vec3(0.1f, 0.1f, 0.2f);
    m_NightLighting.fogDensity = 0.01f;
    m_NightLighting.fogColor = glm::vec3(0.05f, 0.05f, 0.15f);
    
    // Рассвет - оранжевый/розовый
    m_DawnLighting.ambientColor = glm::vec3(0.6f, 0.4f, 0.3f);
    m_DawnLighting.directionalLightColor = glm::vec3(1.0f, 0.6f, 0.4f);
    m_DawnLighting.directionalLightDirection = glm::vec3(-0.5f, -0.5f, -0.5f);
    m_DawnLighting.ambientIntensity = 0.4f;
    m_DawnLighting.directionalIntensity = 0.6f;
    m_DawnLighting.skyColor = glm::vec3(0.8f, 0.5f, 0.3f);
    m_DawnLighting.horizonColor = glm::vec3(1.0f, 0.7f, 0.5f);
    m_DawnLighting.fogDensity = 0.005f;
    m_DawnLighting.fogColor = glm::vec3(0.8f, 0.6f, 0.5f);
    
    // День - яркий, белый свет
    m_DayLighting.ambientColor = glm::vec3(0.8f, 0.8f, 0.9f);
    m_DayLighting.directionalLightColor = glm::vec3(1.0f, 1.0f, 0.95f);
    m_DayLighting.directionalLightDirection = glm::vec3(0.0f, -1.0f, -0.2f);
    m_DayLighting.ambientIntensity = 0.5f;
    m_DayLighting.directionalIntensity = 1.0f;
    m_DayLighting.skyColor = glm::vec3(0.4f, 0.7f, 1.0f);
    m_DayLighting.horizonColor = glm::vec3(0.8f, 0.9f, 1.0f);
    m_DayLighting.fogDensity = 0.0f;
    m_DayLighting.fogColor = glm::vec3(0.7f, 0.8f, 0.9f);
    
    // Закат - красный/оранжевый
    m_DuskLighting.ambientColor = glm::vec3(0.7f, 0.3f, 0.2f);
    m_DuskLighting.directionalLightColor = glm::vec3(1.0f, 0.5f, 0.3f);
    m_DuskLighting.directionalLightDirection = glm::vec3(0.5f, -0.5f, -0.5f);
    m_DuskLighting.ambientIntensity = 0.4f;
    m_DuskLighting.directionalIntensity = 0.6f;
    m_DuskLighting.skyColor = glm::vec3(1.0f, 0.4f, 0.2f);
    m_DuskLighting.horizonColor = glm::vec3(1.0f, 0.6f, 0.3f);
    m_DuskLighting.fogDensity = 0.005f;
    m_DuskLighting.fogColor = glm::vec3(0.9f, 0.5f, 0.4f);
}

json DayNightCycle::ToJson() const {
    return json{
        {"currentTime", m_CurrentTime},
        {"timeScale", m_TimeScale},
        {"dayDuration", m_DayDuration},
        {"paused", m_Paused},
        {"nightLighting", m_NightLighting.ToJson()},
        {"dawnLighting", m_DawnLighting.ToJson()},
        {"dayLighting", m_DayLighting.ToJson()},
        {"duskLighting", m_DuskLighting.ToJson()},
        {"weather", m_Weather.ToJson()}
    };
}

void DayNightCycle::FromJson(const json& j) {
    m_CurrentTime = j.value("currentTime", 12.0f);
    m_TimeScale = j.value("timeScale", 1.0f);
    m_DayDuration = j.value("dayDuration", 600.0f);
    m_Paused = j.value("paused", false);
    
    m_LastHour = GetHour();
    UpdateTimeOfDay();
    
    if (j.contains("nightLighting")) {
        m_NightLighting.FromJson(j["nightLighting"]);
    }
    
    if (j.contains("dawnLighting")) {
        m_DawnLighting.FromJson(j["dawnLighting"]);
    }
    
    if (j.contains("dayLighting")) {
        m_DayLighting.FromJson(j["dayLighting"]);
    }
    
    if (j.contains("duskLighting")) {
        m_DuskLighting.FromJson(j["duskLighting"]);
    }
    
    if (j.contains("weather")) {
        m_Weather.FromJson(j["weather"]);
    }
}

} // namespace SAGE
