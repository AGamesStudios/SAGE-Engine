#include "SAGE.h"
#include "ECS/ECS.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <random>
#include <unordered_map>
#include <vector>
#include <json/json.hpp>
#include <Windows.h>

#include "Math/Matrix4.h"

namespace ECS = SAGE::ECS;
namespace Physics = SAGE::Physics;

using SAGE::CreateRef;
using SAGE::Matrix4;
using SAGE::Ref;
using SAGE::Shader;
using SAGE::Vector2;

namespace {

struct Vertex {
    float x;
    float y;
    float r;
    float g;
    float b;
    float a;
};

struct DebugColor {
    float r;
    float g;
    float b;
    float a;
};

constexpr const char* kVertexShader = R"GLSL(
#version 330 core
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec4 a_Color;
uniform mat4 uProjection;
out vec4 v_Color;
void main() {
    v_Color = a_Color;
    gl_Position = uProjection * vec4(a_Position.xy, 0.0, 1.0);
}
)GLSL";

constexpr const char* kFragmentShader = R"GLSL(
#version 330 core
in vec4 v_Color;
out vec4 FragColor;
void main() {
    FragColor = v_Color;
}
)GLSL";

DebugColor MakeColor(float r, float g, float b, float a = 1.0f) {
    return DebugColor{ std::clamp(r, 0.0f, 1.0f), std::clamp(g, 0.0f, 1.0f), std::clamp(b, 0.0f, 1.0f), std::clamp(a, 0.0f, 1.0f) };
}

Vertex MakeVertex(float x, float y, const DebugColor& c) {
    return Vertex{ x, y, c.r, c.g, c.b, c.a };
}

class PhysicsDemo {
public:
    PhysicsDemo(unsigned width, unsigned height)
        : m_Width(width)
        , m_Height(height)
        , m_RandomEngine(static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count())) {
        
        Physics::Settings settings;
    settings.gravity = Vector2(0.0f, -980.0f);
        settings.globalLinearDamping = 0.998f;      // Слабое глобальное затухание
        settings.globalAngularDamping = 0.950f;     // Сильное угловое затухание
        settings.allowedPenetration = 5.0f;         // Большой slop - меньше микро-импульсов
        settings.positionCorrectionPercent = 0.05f; // Минимальная коррекция 5%
        settings.ccdPenetrationThreshold = 10.0f;
        settings.maxSubSteps = 3;
        settings.minSubSteps = 1;
        settings.solverIterations = 8;
        settings.restitutionVelocityThreshold = 50.0f;
        settings.restitutionVelocitySaturation = 300.0f;
        settings.restitutionScale = 0.03f;
        settings.maxRestitution = 0.03f;            // Почти без отскоков
        settings.defaultStaticFriction = 0.8f;      // Высокое трение
        settings.defaultDynamicFriction = 0.7f;
        settings.restingLinearVelocityThreshold = 50.0f;
        settings.restingLinearDamping = 0.60f;      // Сильное затухание при контакте
        settings.restingAngularVelocityThreshold = 20.0f;
        settings.restingAngularDamping = 0.15f;     // Экстремальное угловое затухание при контакте
        settings.maxAngularVelocity = 6.0f;
        settings.sleepLinearThreshold = 10.0f;      // РЕАЛИСТИЧНО: Учитываем гравитационную скорость (~8-10 px/s)
        settings.sleepAngularThreshold = 0.15f;     // АГРЕССИВНО: Очень низкий порог вращения
        settings.sleepTimeThreshold = 0.2f;         // БАЛАНС: Достаточно времени для стабилизации (0.2s = 12 кадров)
        settings.warmStartThreshold = 0.2f;
        settings.enableDebugLogging = true;
        
        LoadPhysicsSettings(settings);
        m_Physics.SetSettings(settings);
        
        // Enable visual debug
        m_Physics.SetDebugDrawEnabled(true);

        if (m_ConfigDirty) {
            SavePhysicsSettings();
            m_ConfigDirty = false;
        }

        CreateStaticBounds();
        SpawnInitialStack();
    }

    void Update(float deltaTime) {
        constexpr float kFixedStep = 1.0f / 120.0f;
        m_Accumulator += deltaTime;
        m_Accumulator = std::min(m_Accumulator, 0.5f);
        while (m_Accumulator >= kFixedStep) {
            m_Physics.Update(m_Registry, kFixedStep);
            m_Accumulator -= kFixedStep;
        }
    }

    void SpawnDynamicBox(bool allowFullRotation = false) {
        if (m_DynamicCount >= m_MaxDynamicBoxes) {
            return;
        }

        std::uniform_real_distribution<float> widthDist(30.0f, 70.0f);
        std::uniform_real_distribution<float> heightDist(30.0f, 70.0f);
        std::uniform_real_distribution<float> xDist(200.0f, static_cast<float>(m_Width) - 200.0f);
    std::uniform_real_distribution<float> hueDist(0.25f, 0.95f);
    std::uniform_real_distribution<float> satDist(0.45f, 0.85f);
    std::uniform_real_distribution<float> brightDist(0.6f, 0.95f);
        std::uniform_real_distribution<float> limitedTiltDist(-12.0f, 12.0f);
        std::uniform_real_distribution<float> fullRotationDist(0.0f, 360.0f);
        // УМЕНЬШИЛ начальное вращение для allowFullRotation с ±0.9 до ±0.3 rad/s
        std::uniform_real_distribution<float> angularVelocityDist(allowFullRotation ? -0.3f : 0.0f, allowFullRotation ? 0.3f : 0.0f);
        std::uniform_real_distribution<float> horizontalVelocityDist(allowFullRotation ? -90.0f : -20.0f, allowFullRotation ? 90.0f : 20.0f);

        const float boxWidth = widthDist(m_RandomEngine);
        const float boxHeight = heightDist(m_RandomEngine);
        const float posX = xDist(m_RandomEngine);
        const float posY = 80.0f;

        const float spawnRotation = allowFullRotation ? fullRotationDist(m_RandomEngine) : limitedTiltDist(m_RandomEngine);

    ECS::Entity entity = m_Registry.CreateEntity();
    // Если вращение запрещено - начинаем с угла 0 (горизонтально)
    float initialRotation = allowFullRotation ? spawnRotation : 0.0f;
    m_Registry.AddComponent(entity, ECS::TransformComponent(Vector2(posX, posY), initialRotation));
        
        auto& collider = m_Registry.AddComponent<ECS::ColliderComponent>(entity);
        collider.SetBox(boxWidth, boxHeight);

        ECS::PhysicsComponent body;
        body.type = ECS::PhysicsBodyType::Dynamic;
        // Не задаём массу вручную - пусть автомасса работает по площади коллайдера
    body.restitution = 0.01f;  // Минимальная упругость
    body.staticFriction = 0.95f;  // МАКСИМАЛЬНОЕ трение для стабильности
    body.dynamicFriction = 0.85f;  // МАКСИМАЛЬНОЕ динамическое трение
    body.linearDamping = allowFullRotation ? 0.97f : 0.995f;  // Очень слабое линейное
    body.angularDamping = allowFullRotation ? 0.950f : 0.920f;  // УСИЛИЛ с 0.85 до 0.950 для вращающихся объектов
    body.gravityScale = 1.0f;
    body.lockRotation = !allowFullRotation;  // Блокируем вращение если не разрешено
        if (allowFullRotation) {
            body.velocity.x = horizontalVelocityDist(m_RandomEngine);
            body.angularVelocity = angularVelocityDist(m_RandomEngine);
        } else {
            body.velocity.x = 0.0f;
            body.angularVelocity = 0.0f;
        }
        // Убрали специальное переопределение итераций - пусть система сама решает
        m_Registry.AddComponent(entity, body);

        DebugColor color = HSVToRGB(hueDist(m_RandomEngine), satDist(m_RandomEngine), brightDist(m_RandomEngine));
        m_Colors[entity] = color;
        ++m_DynamicCount;

        std::cout << "[Sandbox] Spawned box id=" << static_cast<std::uint32_t>(entity)
                  << " pos=(" << posX << ", " << posY << ")"
                  << " size=(" << boxWidth << "x" << boxHeight << ")"
                  << " rot=" << spawnRotation
                  << " allowFullRot=" << (allowFullRotation ? "true" : "false")
                  << " linVelX=" << body.velocity.x
                  << " angVel=" << body.angularVelocity
                  << std::endl;
    }

    const ECS::Registry& GetRegistry() const { return m_Registry; }
    ECS::Registry& GetRegistry() { return m_Registry; }

    ECS::PhysicsSystem& GetPhysicsSystem() { return m_Physics; }
    const ECS::PhysicsSystem& GetPhysicsSystem() const { return m_Physics; }

    DebugColor GetColorForEntity(ECS::Entity entity, bool isTrigger, bool isStatic) const {
        auto it = m_Colors.find(entity);
        if (it != m_Colors.end()) {
            return it->second;
        }
        if (isTrigger) {
            return MakeColor(1.0f, 0.8f, 0.2f, 0.35f);
        }
        if (isStatic) {
            return MakeColor(0.55f, 0.58f, 0.62f);
        }
        return MakeColor(0.2f, 0.7f, 1.0f);
    }

    void ResetScene() {
        const bool triggerWasActive = m_TriggerActive;
        m_Registry.Clear();
        m_Colors.clear();
        m_DynamicCount = 0;
        m_Accumulator = 0.0f;
        m_TriggerEntity = ECS::NullEntity;
        m_TriggerActive = false;
        CreateStaticBounds();
        SpawnInitialStack();
        if (triggerWasActive) {
            SetTriggerActive(true);
        }
    }

    void AdjustSolverIterations(int delta) {
        Physics::Settings settings = m_Physics.GetWorld().GetSettings();
        const int newValue = std::clamp(settings.solverIterations + delta, 1, 32);
        if (newValue == settings.solverIterations) {
            return;
        }
        settings.solverIterations = newValue;
        m_Physics.SetSettings(settings);
        StoreSolverIterations(newValue);
        std::cout << "[Sandbox] Solver iterations: " << newValue << std::endl;
    }

    static DebugColor HSVToRGB(float h, float s, float v) {
        const float hue = std::fmod(std::max(h, 0.0f), 1.0f) * 6.0f;
        const int sector = static_cast<int>(hue);
        const float fraction = hue - static_cast<float>(sector);
        const float p = v * (1.0f - s);
        const float q = v * (1.0f - s * fraction);
        const float t = v * (1.0f - s * (1.0f - fraction));

        switch (sector) {
            case 0: return MakeColor(v, t, p);
            case 1: return MakeColor(q, v, p);
            case 2: return MakeColor(p, v, t);
            case 3: return MakeColor(p, q, v);
            case 4: return MakeColor(t, p, v);
            default: return MakeColor(v, p, q);
        }
    }

    int GetSolverIterations() const {
        return m_Physics.GetWorld().GetSettings().solverIterations;
    }

    void ToggleTriggerZone() {
        SetTriggerActive(!m_TriggerActive);
    }

    bool IsTriggerActive() const { return m_TriggerActive; }

private:

private:
    void CreateStaticBounds() {
        const float floorHeight = 40.0f;
        const float wallThickness = 40.0f;

        ECS::Entity floor = m_Registry.CreateEntity();
        m_Registry.AddComponent(floor, ECS::TransformComponent(Vector2(0.0f, static_cast<float>(m_Height) - floorHeight)));
        auto& floorCollider = m_Registry.AddComponent<ECS::ColliderComponent>(floor);
        floorCollider.SetBox(static_cast<float>(m_Width), floorHeight);
        ECS::PhysicsComponent floorBody;
        floorBody.type = ECS::PhysicsBodyType::Static;
    floorBody.restitution = 0.05f;
    floorBody.staticFriction = 0.6f;
    floorBody.dynamicFriction = 0.45f;
        m_Registry.AddComponent(floor, floorBody);
        m_Colors[floor] = MakeColor(0.35f, 0.37f, 0.40f);

        ECS::Entity leftWall = m_Registry.CreateEntity();
        m_Registry.AddComponent(leftWall, ECS::TransformComponent(Vector2(-wallThickness, 0.0f)));
        m_Registry.AddComponent(leftWall, ECS::ColliderComponent(Vector2(wallThickness, static_cast<float>(m_Height))));
        ECS::PhysicsComponent leftBody;
        leftBody.type = ECS::PhysicsBodyType::Static;
        m_Registry.AddComponent(leftWall, leftBody);
        m_Colors[leftWall] = MakeColor(0.30f, 0.32f, 0.34f);

        ECS::Entity rightWall = m_Registry.CreateEntity();
        m_Registry.AddComponent(rightWall, ECS::TransformComponent(Vector2(static_cast<float>(m_Width), 0.0f)));
        m_Registry.AddComponent(rightWall, ECS::ColliderComponent(Vector2(wallThickness, static_cast<float>(m_Height))));
        ECS::PhysicsComponent rightBody;
        rightBody.type = ECS::PhysicsBodyType::Static;
        m_Registry.AddComponent(rightWall, rightBody);
        m_Colors[rightWall] = MakeColor(0.30f, 0.32f, 0.34f);

    ECS::Entity platform = m_Registry.CreateEntity();
    m_Registry.AddComponent(platform, ECS::TransformComponent(Vector2(static_cast<float>(m_Width) * 0.4f, static_cast<float>(m_Height) * 0.55f)));
    m_Registry.AddComponent(platform, ECS::ColliderComponent(Vector2(200.0f, 30.0f)));
    ECS::PhysicsComponent platformBody;
    platformBody.type = ECS::PhysicsBodyType::Static;
    platformBody.restitution = 0.06f;
    platformBody.staticFriction = 0.55f;
    platformBody.dynamicFriction = 0.4f;
    m_Registry.AddComponent(platform, platformBody);
    m_Colors[platform] = MakeColor(0.45f, 0.28f, 0.05f, 1.0f);
    }

    void SpawnInitialStack() {
        for (int i = 0; i < 2; ++i) {  // Временно 2 объекта для тестирования столкновений
            SpawnDynamicBox();
        }
    }

    void SetTriggerActive(bool active) {
        if (active == m_TriggerActive) {
            return;
        }

        if (!active) {
            if (m_TriggerEntity != ECS::NullEntity) {
                m_Registry.DestroyEntity(m_TriggerEntity);
            }
            m_Colors.erase(m_TriggerEntity);
            m_TriggerEntity = ECS::NullEntity;
            m_TriggerActive = false;
            std::cout << "[Sandbox] Trigger platform disabled" << std::endl;
            return;
        }

    m_TriggerEntity = m_Registry.CreateEntity();
    const Vector2 triggerPosition(static_cast<float>(m_Width) * 0.65f, static_cast<float>(m_Height) * 0.35f);
    m_Registry.AddComponent(m_TriggerEntity, ECS::TransformComponent(triggerPosition));
    ECS::ColliderComponent triggerCollider(Vector2(160.0f, 24.0f));
        triggerCollider.isTrigger = true;
        m_Registry.AddComponent(m_TriggerEntity, triggerCollider);
        ECS::PhysicsComponent triggerBody;
        triggerBody.type = ECS::PhysicsBodyType::Static;
        m_Registry.AddComponent(m_TriggerEntity, triggerBody);
        m_Colors[m_TriggerEntity] = MakeColor(1.0f, 0.6f, 0.0f, 0.35f);
        m_TriggerActive = true;
        std::cout << "[Sandbox] Trigger platform enabled" << std::endl;
    }
private:
    void LoadPhysicsSettings(Physics::Settings& settings) {
        using nlohmann::json;

        m_ConfigData = json::object();

        std::ifstream file(m_ConfigPath);
        if (file.is_open()) {
            try {
                file >> m_ConfigData;
            } catch (const std::exception& e) {
                std::cerr << "[Sandbox] Failed to parse " << m_ConfigPath << ": " << e.what() << std::endl;
                m_ConfigData = json::object();
            }
        }

        if (!m_ConfigData.is_object()) {
            m_ConfigData = json::object();
        }

        json& physicsNode = m_ConfigData["physics"];
        if (!physicsNode.is_object()) {
            physicsNode = json::object();
            m_ConfigDirty = true;
        }

        if (physicsNode.contains("solverIterations") && physicsNode["solverIterations"].is_number()) {
            settings.solverIterations = std::clamp(physicsNode["solverIterations"].get<int>(), 1, 32);
        } else {
            physicsNode["solverIterations"] = settings.solverIterations;
            m_ConfigDirty = true;
        }
    }

    void StoreSolverIterations(int iterations) {
        using nlohmann::json;

        if (!m_ConfigData.is_object()) {
            m_ConfigData = json::object();
        }

        json& physicsNode = m_ConfigData["physics"];
        if (!physicsNode.is_object()) {
            physicsNode = json::object();
        }

        const int current = physicsNode.contains("solverIterations") && physicsNode["solverIterations"].is_number()
            ? physicsNode["solverIterations"].get<int>()
            : -1;
        if (current == iterations) {
            return;
        }

        physicsNode["solverIterations"] = iterations;
        SavePhysicsSettings();
    }

    void SavePhysicsSettings() {
        if (!m_ConfigData.is_object()) {
            return;
        }

        std::ofstream file(m_ConfigPath, std::ios::trunc);
        if (!file.is_open()) {
            std::cerr << "[Sandbox] Unable to write " << m_ConfigPath << std::endl;
            return;
        }

        try {
            file << m_ConfigData.dump(4);
        } catch (const std::exception& e) {
            std::cerr << "[Sandbox] Failed to write " << m_ConfigPath << ": " << e.what() << std::endl;
        }
    }

    ECS::Registry m_Registry;
    ECS::PhysicsSystem m_Physics;
    std::unordered_map<ECS::Entity, DebugColor> m_Colors;
    std::mt19937 m_RandomEngine;
    ECS::Entity m_TriggerEntity = ECS::NullEntity;
    bool m_TriggerActive = false;

    const unsigned m_Width;
    const unsigned m_Height;
    float m_Accumulator = 0.0f;
    int m_DynamicCount = 0;
    static constexpr int m_MaxDynamicBoxes = 50;
    std::string m_ConfigPath = "engine_config.json";
    nlohmann::json m_ConfigData;
    bool m_ConfigDirty = false;
};

void SubmitRectangleVertices(std::vector<Vertex>& outVertices, const std::array<Vector2, 4>& verts, const DebugColor& color) {
    outVertices.push_back(MakeVertex(verts[0].x, verts[0].y, color));
    outVertices.push_back(MakeVertex(verts[1].x, verts[1].y, color));
    outVertices.push_back(MakeVertex(verts[2].x, verts[2].y, color));

    outVertices.push_back(MakeVertex(verts[0].x, verts[0].y, color));
    outVertices.push_back(MakeVertex(verts[2].x, verts[2].y, color));
    outVertices.push_back(MakeVertex(verts[3].x, verts[3].y, color));
}

void SubmitLine(std::vector<Vertex>& outVertices, const Vector2& a, const Vector2& b, const DebugColor& color) {
    outVertices.push_back(MakeVertex(a.x, a.y, color));
    outVertices.push_back(MakeVertex(b.x, b.y, color));
}

void SubmitCross(std::vector<Vertex>& outVertices, const Vector2& center, float halfSize, const DebugColor& color) {
    const Vector2 left(center.x - halfSize, center.y);
    const Vector2 right(center.x + halfSize, center.y);
    const Vector2 top(center.x, center.y - halfSize);
    const Vector2 bottom(center.x, center.y + halfSize);
    SubmitLine(outVertices, left, right, color);
    SubmitLine(outVertices, top, bottom, color);
}

} // namespace

int main() {
    constexpr unsigned kWindowWidth = 1280;
    constexpr unsigned kWindowHeight = 720;

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(static_cast<int>(kWindowWidth), static_cast<int>(kWindowHeight), "SAGE Physics Sandbox", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to load OpenGL functions" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Ref<Shader> shader = CreateRef<Shader>(kVertexShader, kFragmentShader);
    if (!shader || !shader->IsValid()) {
        std::cerr << "Failed to compile sandbox shaders" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    GLuint vao = 0;
    GLuint vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(2 * sizeof(float)));
    glBindVertexArray(0);

    const Matrix4 projection = Matrix4::Orthographic(0.0f, static_cast<float>(kWindowWidth), static_cast<float>(kWindowHeight), 0.0f, -1.0f, 1.0f);

    PhysicsDemo demo(kWindowWidth, kWindowHeight);

    auto previousTime = std::chrono::steady_clock::now();
    double elapsedSeconds = 0.0;
    int frameCounter = 0;
    bool increaseIterationsHeld = false;
    bool decreaseIterationsHeld = false;
    bool toggleTriggerHeld = false;

    std::cout << "Controls: [Space] spawn random box | [R] reset scene | [T] toggle trigger | '['/']' adjust solver iterations | [Esc] exit" << std::endl;
    std::cout << "💡 DEBUG: Physics debug logging is ON - checking for output every 3 seconds..." << std::endl;

    while (!glfwWindowShouldClose(window)) {
        auto now = std::chrono::steady_clock::now();
        const float deltaTime = std::chrono::duration<float>(now - previousTime).count();
        previousTime = now;

        demo.Update(deltaTime);

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            demo.SpawnDynamicBox(true);
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            demo.ResetScene();
        }
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        const bool increaseDown = glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS;
        if (increaseDown && !increaseIterationsHeld) {
            demo.AdjustSolverIterations(1);
        }
        increaseIterationsHeld = increaseDown;

        const bool decreaseDown = glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS;
        if (decreaseDown && !decreaseIterationsHeld) {
            demo.AdjustSolverIterations(-1);
        }
        decreaseIterationsHeld = decreaseDown;

        const bool toggleTriggerDown = glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS;
        if (toggleTriggerDown && !toggleTriggerHeld) {
            demo.ToggleTriggerZone();
        }
        toggleTriggerHeld = toggleTriggerDown;

        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.07f, 0.08f, 0.11f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader->Bind();
        shader->SetMat4("uProjection", projection.Data());

        std::vector<Vertex> vertices;
        vertices.reserve(1024);

        auto colliders = demo.GetRegistry().GetAllWith<ECS::ColliderComponent>();
        for (auto& entry : colliders) {
            ECS::Entity entity = entry.entity;
            auto& collider = entry.Get();
            auto* transform = demo.GetRegistry().GetComponent<ECS::TransformComponent>(entity);
            auto* body = demo.GetRegistry().GetComponent<ECS::PhysicsComponent>(entity);
            if (!transform || !body) {
                continue;
            }
            std::array<Vector2, 4> verts;
            collider.GetWorldVertices(*transform, verts);
            const bool isStatic = (body->type == ECS::PhysicsBodyType::Static);
            const DebugColor color = demo.GetColorForEntity(entity, collider.isTrigger, isStatic);
            SubmitRectangleVertices(vertices, verts, color);
        }

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)), vertices.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));

        std::vector<Vertex> contactVertices;
        const auto& contacts = demo.GetPhysicsSystem().GetWorld().GetContacts();
        contactVertices.reserve(std::max<size_t>(contacts.size() * 8, 32));
        for (const auto& contact : contacts) {
            if (contact.normal.LengthSquared() < 1e-6f) {
                continue;
            }

            const DebugColor lineColor = contact.isTrigger ? MakeColor(1.0f, 0.95f, 0.2f) : MakeColor(1.0f, 0.25f, 0.25f);
            const DebugColor pointColor = contact.isTrigger ? MakeColor(1.0f, 0.75f, 0.1f) : MakeColor(1.0f, 0.45f, 0.45f);

            const Vector2 normalDir = contact.normal.Normalized();
            const float rayLength = contact.isTrigger ? 30.0f : 40.0f;

            if (!contact.contactPoints.empty()) {
                for (const Vector2& point : contact.contactPoints) {
                    SubmitLine(contactVertices, point, point + normalDir * rayLength, lineColor);
                    SubmitCross(contactVertices, point, 4.0f, pointColor);
                }
            } else {
                // Fallback: midpoint between shapes if narrow phase did not provide explicit points
                const auto* transformA = demo.GetRegistry().GetComponent<ECS::TransformComponent>(contact.entityA);
                const auto* colliderA = demo.GetRegistry().GetComponent<ECS::ColliderComponent>(contact.entityA);
                const auto* transformB = demo.GetRegistry().GetComponent<ECS::TransformComponent>(contact.entityB);
                const auto* colliderB = demo.GetRegistry().GetComponent<ECS::ColliderComponent>(contact.entityB);
                if (!transformA || !colliderA || !transformB || !colliderB) {
                    continue;
                }
                const Vector2 centerA = colliderA->GetCenter(*transformA);
                const Vector2 centerB = colliderB->GetCenter(*transformB);
                const Vector2 contactPoint = (centerA + centerB) * 0.5f;
                SubmitLine(contactVertices, contactPoint, contactPoint + normalDir * rayLength, lineColor);
                SubmitCross(contactVertices, contactPoint, 4.0f, pointColor);
            }
        }

        if (!contactVertices.empty()) {
            glLineWidth(2.0f);
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(contactVertices.size() * sizeof(Vertex)), contactVertices.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(contactVertices.size()));
        }

        // ✨ VISUAL DEBUG: Trails, Velocity Vectors, Collision Points
        const auto& debugDrawData = demo.GetPhysicsSystem().GetWorld().GetDebugDrawData();
        std::vector<Vertex> debugVertices;
        debugVertices.reserve(2048);
        
        // 1. Draw trails (path history) - каждая entity свой цвет с градиентом
        std::unordered_map<ECS::Entity, DebugColor> trailColors;
        float hue = 0.0f;
        for (const auto& [entity, trail] : debugDrawData.trails) {
            if (trail.size() < 2) continue;
            
            // Generate unique color for each entity
            if (trailColors.find(entity) == trailColors.end()) {
                trailColors[entity] = PhysicsDemo::HSVToRGB(hue, 0.8f, 0.9f);
                hue += 0.618033988749895f; // Golden ratio for color distribution
                if (hue > 1.0f) hue -= 1.0f;
            }
            
            const auto& baseColor = trailColors[entity];
            
            // Draw trail with fade effect (older points more transparent)
            const float currentTime = debugDrawData.trails.begin()->second.back().timestamp;
            for (size_t i = 1; i < trail.size(); ++i) {
                const float age = currentTime - trail[i].timestamp;
                const float alpha = std::max(0.2f, 1.0f - (age / debugDrawData.maxTrailAge));
                
                DebugColor fadeColor = baseColor;
                fadeColor.a = alpha;
                
                SubmitLine(debugVertices, trail[i-1].position, trail[i].position, fadeColor);
            }
        }
        
        // 2. Draw velocity vectors from pivot point (center of collider)
        for (const auto& [entity, velocity] : debugDrawData.velocities) {
            auto* transform = demo.GetRegistry().GetComponent<ECS::TransformComponent>(entity);
            auto* collider = demo.GetRegistry().GetComponent<ECS::ColliderComponent>(entity);
            if (!transform || !collider || velocity.LengthSquared() < 1.0f) continue;
            
            // Use collider center as pivot point (accounts for offset and size)
            const Vector2 center = collider->GetCenter(*transform);
            
            const float scale = 0.15f; // Scale down velocity visualization
            const Vector2 end = center + velocity * scale;
            const DebugColor velColor = MakeColor(0.0f, 1.0f, 0.0f, 0.8f); // Green
            
            SubmitLine(debugVertices, center, end, velColor);
            
            // Arrow head
            Vector2 dir = (end - center).Normalized();
            Vector2 perp(-dir.y, dir.x);
            Vector2 arrowTip1 = end - dir * 8.0f + perp * 4.0f;
            Vector2 arrowTip2 = end - dir * 8.0f - perp * 4.0f;
            SubmitLine(debugVertices, end, arrowTip1, velColor);
            SubmitLine(debugVertices, end, arrowTip2, velColor);
            
            // Draw pivot point as white dot (3px cross)
            const DebugColor pivotColor = MakeColor(1.0f, 1.0f, 1.0f, 0.9f); // White
            SubmitCross(debugVertices, center, 3.0f, pivotColor);
        }
        
        // 3. Draw collision points and normals
        for (const auto& cp : debugDrawData.collisionPoints) {
            const DebugColor cpColor = MakeColor(1.0f, 0.0f, 0.0f, 0.9f); // Red
            const DebugColor normalColor = MakeColor(1.0f, 1.0f, 0.0f, 0.8f); // Yellow
            
            // Draw collision point
            SubmitCross(debugVertices, cp.position, 6.0f, cpColor);
            
            // Draw normal
            const Vector2 normalEnd = cp.position + cp.normal * 30.0f;
            SubmitLine(debugVertices, cp.position, normalEnd, normalColor);
        }
        
        if (!debugVertices.empty()) {
            glLineWidth(2.5f);
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(debugVertices.size() * sizeof(Vertex)), debugVertices.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(debugVertices.size()));
        }

        glBindVertexArray(0);

        // 🔍 ДЕТАЛЬНЫЙ DEBUG КАЖДЫЙ КАДР
        static int frameDebugCounter = 0;
        frameDebugCounter++;
        
        const auto& debugInfo = demo.GetPhysicsSystem().GetWorld().GetDebugInfo();
        
        if (frameDebugCounter % 60 == 0) {  // Каждую секунду (при 60 FPS)
            std::cout << "\n╔════════════════════════════════════════════════════════════╗" << std::endl;
            std::cout << "║ PHYSICS DEBUG FRAME #" << frameDebugCounter 
                      << " @ " << std::fixed << std::setprecision(2) << glfwGetTime() << "s" << std::endl;
            std::cout << "╠════════════════════════════════════════════════════════════╣" << std::endl;
            
            if (!debugInfo.empty()) {
                int awakeCount = 0;
                int sleepingCount = 0;
                for (const auto& info : debugInfo) {
                    if (info.isSleeping) sleepingCount++;
                    else awakeCount++;
                }
                std::cout << "║ Bodies: " << debugInfo.size() 
                          << " | Awake: " << awakeCount 
                          << " | Sleeping: " << sleepingCount 
                          << " | Contacts: " << contacts.size() << std::endl;
                std::cout << "╠════════════════════════════════════════════════════════════╣" << std::endl;
                
                // Показать ВСЕ динамические тела с деталями
                for (const auto& info : debugInfo) {
                    // Пропускаем статические границы (entity 1-4)
                    if (static_cast<int>(info.entity) <= 4) continue;
                    
                    std::cout << "║ Entity " << std::setw(2) << static_cast<int>(info.entity) << " | ";
                    
                    if (info.isSleeping) {
                        std::cout << "💤 SLEEPING";
                    } else {
                        std::cout << "⚡ AWAKE   ";
                    }
                    
                    std::cout << " | Pos(" << std::fixed << std::setprecision(1) << std::setw(6) << info.position.x 
                              << "," << std::setw(6) << info.position.y << ") ";
                    std::cout << "Vel(" << std::setprecision(1) << std::setw(5) << info.velocity.x 
                              << "," << std::setw(5) << info.velocity.y << ") ";
                    std::cout << "ω=" << std::setprecision(2) << std::setw(5) << info.angularVelocity << std::endl;
                    
                    std::cout << "║          | ";
                    std::cout << "Contacts:" << std::setw(2) << info.contactCount << " ";
                    std::ostringstream impulseStream;
                    impulseStream << std::fixed << std::setprecision(3) << info.totalImpulseApplied;
                    std::cout << "Impulse:" << std::setw(8) << impulseStream.str() << " ";
                    std::cout << "Mass:" << std::setprecision(2) << std::setw(6) << info.mass << " ";
                    std::cout << "Sleep:" << std::setprecision(2) << std::setw(4) << info.sleepTimer << "s ";
                    
                    if (info.hadPositionCorrection) std::cout << "[PosCorr] ";
                    if (info.hadRestingDamping) std::cout << "[RestDamp] ";
                    std::cout << std::endl;
                }
                std::cout << "╚════════════════════════════════════════════════════════════╝\n" << std::endl;
            } else {
                std::cout << "║ ⚠️  WARNING: DebugInfo is EMPTY!" << std::endl;
                std::cout << "╚════════════════════════════════════════════════════════════╝\n" << std::endl;
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        elapsedSeconds += deltaTime;
        ++frameCounter;
        if (elapsedSeconds >= 1.0) {
            const double fps = static_cast<double>(frameCounter) / elapsedSeconds;
            const auto& profile = demo.GetPhysicsSystem().GetWorld().GetLastProfile();
            std::cout << "FPS: " << fps 
                      << " | substeps: " << profile.subSteps
                      << " | contacts: " << contacts.size() 
                      << " | iterations: " << demo.GetSolverIterations() << std::endl;
            
            frameCounter = 0;
            elapsedSeconds = 0.0;
        }
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
