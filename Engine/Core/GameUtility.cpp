#include "GameUtility.h"
#include "GameObject.h"
#include "Scene.h"
#include "Logger.h"
#include "ServiceLocator.h"
#include "SpatialHashGrid.h"
#include "../Audio/AudioSystem.h"
#include "../Graphics/Core/Camera2D.h"
#include "../Math/Constants.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <cmath>
#include <map>
#include <algorithm>

namespace SAGE {
namespace GameUtility {

// ============================================================================
// Internal State
// ============================================================================

namespace {
    // Time tracking
    std::chrono::steady_clock::time_point g_StartTime = std::chrono::steady_clock::now();
    float g_DeltaTime = 0.016f;
    float g_FixedDeltaTime = 0.02f;
    uint64_t g_FrameCount = 0;
    
    // Input state
    GLFWwindow* g_Window = nullptr;
    Vector2 g_LastMousePos = Vector2::Zero();
    Vector2 g_MouseDelta = Vector2::Zero();
    
    // Random state
    std::random_device g_RandomDevice;
    std::mt19937 g_RandomEngine(g_RandomDevice());
    std::uniform_real_distribution<float> g_RandomDist(0.0f, 1.0f);
    
    // Delayed destruction
    struct DelayedDestroy {
        GameObject* object;
        float timer;
    };
    std::vector<DelayedDestroy> g_DelayedDestroys;
    
    // Animation system
    struct Animation {
        GameObject* object;
        enum Type { FADE, SCALE, ROTATE, MOVE } type;
        float startValue;
        float targetValue;
        Vector2 startPos;
        Vector2 targetPos;
        float duration;
        float elapsed;
        bool easeIn;
    };
    std::vector<Animation> g_Animations;
    
    // Camera shake system
    struct CameraShake {
        Camera2D* camera;
        float intensity;
        float duration;
        float elapsed;
        Vector2 originalPos;
    };
    std::vector<CameraShake> g_CameraShakes;
    
    // Spatial optimization
    SpatialHashGrid g_SpatialGrid(128.0f); // 128 pixels per cell
    bool g_SpatialGridDirty = true;
}

// ============================================================================
// Initialization (should be called by engine)
// ============================================================================

void Initialize(GLFWwindow* window) {
    g_Window = window;
    g_StartTime = std::chrono::steady_clock::now();
}

void UpdateTimeAndInput(float deltaTime) {
    g_DeltaTime = deltaTime;
    g_FrameCount++;
    
    // Rebuild spatial grid if needed (every frame for dynamic objects)
    if (g_SpatialGridDirty) {
        g_SpatialGrid.Rebuild(GameObject::GetAllObjects());
        g_SpatialGridDirty = false;
    }
    
    // Update delayed destroys
    for (auto it = g_DelayedDestroys.begin(); it != g_DelayedDestroys.end();) {
        it->timer -= deltaTime;
        if (it->timer <= 0.0f) {
            if (it->object) {
                it->object->Destroy();
            }
            it = g_DelayedDestroys.erase(it);
        } else {
            ++it;
        }
    }
    
    // Update animations
    for (auto it = g_Animations.begin(); it != g_Animations.end();) {
        it->elapsed += deltaTime;
        if (it->elapsed >= it->duration || !it->object) {
            it = g_Animations.erase(it);
            continue;
        }
        
        float t = it->elapsed / it->duration;
        if (it->easeIn) {
            t = t * t; // Ease-in quadratic
        }
        
        switch (it->type) {
            case Animation::FADE:
                it->object->alpha = Lerp(it->startValue, it->targetValue, t);
                break;
            case Animation::SCALE:
                // Scale would need to be stored in GameObject
                break;
            case Animation::ROTATE:
                it->object->angle = Lerp(it->startValue, it->targetValue, t);
                break;
            case Animation::MOVE:
                it->object->x = Lerp(it->startPos.x, it->targetPos.x, t);
                it->object->y = Lerp(it->startPos.y, it->targetPos.y, t);
                break;
        }
        
        ++it;
    }
    
    // Update camera shakes
    for (auto it = g_CameraShakes.begin(); it != g_CameraShakes.end();) {
        it->elapsed += deltaTime;
        if (it->elapsed >= it->duration || !it->camera) {
            // Restore original position
            if (it->camera) {
                it->camera->SetPosition(it->originalPos);
            }
            it = g_CameraShakes.erase(it);
            continue;
        }
        
        // Apply shake offset
        float shakeAmount = it->intensity * (1.0f - it->elapsed / it->duration);
        Vector2 offset(
            Random::Range(-shakeAmount, shakeAmount),
            Random::Range(-shakeAmount, shakeAmount)
        );
        it->camera->SetPosition(it->originalPos + offset);
        
        ++it;
    }
    
    // Update mouse delta
    if (g_Window) {
        double mouseX, mouseY;
        glfwGetCursorPos(g_Window, &mouseX, &mouseY);
        Vector2 currentMousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));
        g_MouseDelta = currentMousePos - g_LastMousePos;
        g_LastMousePos = currentMousePos;
    }
}

// ============================================================================
// GameObject Management
// ============================================================================

GameObject* Instantiate(GameObject* original, const Vector2& position) {
    if (!original) {
        SAGE_ERROR("GameUtility::Instantiate - original is nullptr");
        return nullptr;
    }
    
    GameObject* clone = GameObject::Create(original->name + "_Clone");
    
    // Copy basic properties
    clone->x = position.x;
    clone->y = position.y;
    clone->angle = original->angle;
    clone->width = original->width;
    clone->height = original->height;
    clone->speedX = original->speedX;
    clone->speedY = original->speedY;
    clone->gravity = original->gravity;
    clone->friction = original->friction;
    clone->bounce = original->bounce;
    clone->physics = original->physics;
    clone->maxFallSpeed = original->maxFallSpeed;
    clone->mass = original->mass;
    clone->gravityScale = original->gravityScale;
    
    // Copy visual properties
    clone->image = original->image;
    clone->color = original->color;
    clone->alpha = original->alpha;
    clone->visible = original->visible;
    clone->flipX = original->flipX;
    clone->flipY = original->flipY;
    
    // Copy collision properties
    clone->collision = original->collision;
    clone->solid = original->solid;
    clone->isTrigger = original->isTrigger;
    clone->hitboxType = original->hitboxType;
    
    // Copy callbacks
    clone->OnCreate = original->OnCreate;
    clone->OnUpdate = original->OnUpdate;
    clone->OnCollision = original->OnCollision;
    clone->OnCollisionEnter = original->OnCollisionEnter;
    clone->OnCollisionStay = original->OnCollisionStay;
    clone->OnCollisionExit = original->OnCollisionExit;
    clone->OnTriggerEnter = original->OnTriggerEnter;
    clone->OnTriggerStay = original->OnTriggerStay;
    clone->OnTriggerExit = original->OnTriggerExit;
    clone->OnDestroy = original->OnDestroy;
    
    SAGE_INFO("Instantiated '{}' at ({}, {})", clone->name, position.x, position.y);
    return clone;
}

std::vector<GameObject*> FindGameObjectsWithTag(const std::string& tag) {
    std::vector<GameObject*> result;
    
    const auto& allObjects = GameObject::GetAllObjects();
    for (GameObject* obj : allObjects) {
        if (obj && obj->active) {
            // Search for tag in name (simple implementation)
            if (obj->name.find(tag) != std::string::npos) {
                result.push_back(obj);
            }
        }
    }
    
    return result;
}

GameObject* FindGameObjectByName(const std::string& name) {
    const auto& allObjects = GameObject::GetAllObjects();
    for (GameObject* obj : allObjects) {
        if (obj && obj->name == name) {
            return obj;
        }
    }
    
    return nullptr;
}

GameObject* FindGameObjectWithTag(const std::string& tag) {
    auto objects = FindGameObjectsWithTag(tag);
    return objects.empty() ? nullptr : objects[0];
}

void Destroy(GameObject* obj, float delay) {
    if (!obj) return;
    
    if (delay <= 0.0f) {
        obj->Destroy();
    } else {
        g_DelayedDestroys.push_back({obj, delay});
    }
}

// ============================================================================
// Input Helpers
// ============================================================================

float GetAxis(const std::string& axisName) {
    if (!g_Window) return 0.0f;
    
    if (axisName == "Horizontal") {
        float value = 0.0f;
        if (glfwGetKey(g_Window, GLFW_KEY_D) == GLFW_PRESS || 
            glfwGetKey(g_Window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            value += 1.0f;
        }
        if (glfwGetKey(g_Window, GLFW_KEY_A) == GLFW_PRESS || 
            glfwGetKey(g_Window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            value -= 1.0f;
        }
        return value;
    }
    else if (axisName == "Vertical") {
        float value = 0.0f;
        if (glfwGetKey(g_Window, GLFW_KEY_W) == GLFW_PRESS || 
            glfwGetKey(g_Window, GLFW_KEY_UP) == GLFW_PRESS) {
            value += 1.0f;
        }
        if (glfwGetKey(g_Window, GLFW_KEY_S) == GLFW_PRESS || 
            glfwGetKey(g_Window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            value -= 1.0f;
        }
        return value;
    }
    
    return 0.0f;
}

bool GetButtonDown(const std::string& buttonName) {
    if (!g_Window) return false;
    
    // Simplified - would need frame-by-frame tracking for true "Down" detection
    return GetButton(buttonName);
}

bool GetButton(const std::string& buttonName) {
    if (!g_Window) return false;
    
    if (buttonName == "Jump") {
        return glfwGetKey(g_Window, GLFW_KEY_SPACE) == GLFW_PRESS;
    }
    else if (buttonName == "Fire" || buttonName == "Fire1") {
        return glfwGetMouseButton(g_Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    }
    else if (buttonName == "Fire2") {
        return glfwGetMouseButton(g_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    }
    else if (buttonName == "Interact") {
        return glfwGetKey(g_Window, GLFW_KEY_E) == GLFW_PRESS;
    }
    
    return false;
}

Vector2 GetMouseDelta() {
    return g_MouseDelta;
}

Vector2 GetMouseWorldPosition(Camera2D* camera) {
    if (!g_Window) {
        SAGE_WARN("GetMouseWorldPosition: No window initialized");
        return Vector2::Zero();
    }
    
    double mouseX, mouseY;
    glfwGetCursorPos(g_Window, &mouseX, &mouseY);
    
    if (camera) {
        return camera->ScreenToWorld(Vector2(static_cast<float>(mouseX), static_cast<float>(mouseY)));
    }
    
    // No camera - return screen coordinates
    return Vector2(static_cast<float>(mouseX), static_cast<float>(mouseY));
}

// ============================================================================
// Time Management
// ============================================================================

float GetDeltaTime() {
    return g_DeltaTime;
}

float GetTimeSinceStartup() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_StartTime);
    return duration.count() / 1000.0f;
}

uint64_t GetFrameCount() {
    return g_FrameCount;
}

float GetFixedDeltaTime() {
    return g_FixedDeltaTime;
}

// ============================================================================
// Math Utilities
// ============================================================================

float SmoothDamp(float current, float target, float& currentVelocity, 
                 float smoothTime, float maxSpeed, float deltaTime) {
    if (deltaTime < 0.0f) {
        deltaTime = g_DeltaTime;
    }
    
    smoothTime = std::max(0.0001f, smoothTime);
    float omega = 2.0f / smoothTime;
    float x = omega * deltaTime;
    float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
    float change = current - target;
    float originalTo = target;
    
    float maxChange = maxSpeed * smoothTime;
    change = Math::Clamp(change, -maxChange, maxChange);
    target = current - change;
    
    float temp = (currentVelocity + omega * change) * deltaTime;
    currentVelocity = (currentVelocity - omega * temp) * exp;
    float output = target + (change + temp) * exp;
    
    if (originalTo - current > 0.0f == output > originalTo) {
        output = originalTo;
        currentVelocity = (output - originalTo) / deltaTime;
    }
    
    return output;
}

Vector2 SmoothDamp(const Vector2& current, const Vector2& target, 
                   Vector2& currentVelocity, float smoothTime, 
                   float maxSpeed, float deltaTime) {
    float x = SmoothDamp(current.x, target.x, currentVelocity.x, smoothTime, maxSpeed, deltaTime);
    float y = SmoothDamp(current.y, target.y, currentVelocity.y, smoothTime, maxSpeed, deltaTime);
    return Vector2(x, y);
}

float MoveTowards(float current, float target, float maxDelta) {
    if (std::abs(target - current) <= maxDelta) {
        return target;
    }
    return current + std::copysign(maxDelta, target - current);
}

Vector2 MoveTowards(const Vector2& current, const Vector2& target, float maxDelta) {
    Vector2 diff = target - current;
    float distance = diff.Length();
    
    if (distance <= maxDelta || distance < 1e-6f) {
        return target;
    }
    
    return current + diff / distance * maxDelta;
}

// ============================================================================
// Random Utilities
// ============================================================================

namespace Random {

float Range(float min, float max) {
    return min + g_RandomDist(g_RandomEngine) * (max - min);
}

int Range(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(g_RandomEngine);
}

float Value() {
    return g_RandomDist(g_RandomEngine);
}

Vector2 InsideUnitCircle() {
    float angle = Range(0.0f, 2.0f * Math::Constants::PI);
    float radius = std::sqrt(Value());
    return Vector2(std::cos(angle) * radius, std::sin(angle) * radius);
}

Vector2 OnUnitCircle() {
    float angle = Range(0.0f, 2.0f * Math::Constants::PI);
    return Vector2(std::cos(angle), std::sin(angle));
}

void SetSeed(unsigned int seed) {
    g_RandomEngine.seed(seed);
}

} // namespace Random

// ============================================================================
// Screen & Display
// ============================================================================

int GetScreenWidth() {
    if (!g_Window) return 0;
    int width, height;
    glfwGetWindowSize(g_Window, &width, &height);
    return width;
}

int GetScreenHeight() {
    if (!g_Window) return 0;
    int width, height;
    glfwGetWindowSize(g_Window, &width, &height);
    return height;
}

void SetFullscreen(bool fullscreen) {
    if (!g_Window) return;
    
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    
    if (fullscreen) {
        glfwSetWindowMonitor(g_Window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        int width = 1280;
        int height = 720;
        glfwSetWindowMonitor(g_Window, nullptr, 100, 100, width, height, 0);
    }
}

bool IsFullscreen() {
    if (!g_Window) return false;
    return glfwGetWindowMonitor(g_Window) != nullptr;
}

void SetVSync(bool enabled) {
    glfwSwapInterval(enabled ? 1 : 0);
}

// ============================================================================
// Camera Utilities
// ============================================================================

void ShakeCamera(Camera2D* camera, float intensity, float duration) {
    if (!camera) return;
    
    CameraShake shake;
    shake.camera = camera;
    shake.intensity = intensity;
    shake.duration = duration;
    shake.elapsed = 0.0f;
    shake.originalPos = camera->GetPosition();
    
    g_CameraShakes.push_back(shake);
}

void CameraFollowTarget(Camera2D* camera, const Vector2& target, float smoothSpeed) {
    if (!camera) return;
    
    Vector2 currentPos = camera->GetPosition();
    Vector2 newPos = Lerp(currentPos, target, smoothSpeed);
    camera->SetPosition(newPos);
}

void SetCameraBounds(Camera2D* camera, float minX, float minY, float maxX, float maxY) {
    if (!camera) return;
    
    Vector2 pos = camera->GetPosition();
    pos.x = Math::Clamp(pos.x, minX, maxX);
    pos.y = Math::Clamp(pos.y, minY, maxY);
    camera->SetPosition(pos);
}

// ============================================================================
// Physics Utilities
// ============================================================================

void ApplyImpulse(GameObject* obj, const Vector2& force) {
    if (!obj) return;
    obj->speedX += force.x;
    obj->speedY += force.y;
}

void ApplyForce(GameObject* obj, const Vector2& direction, float strength) {
    if (!obj) return;
    Vector2 normalized = direction.Normalized();
    obj->speedX += normalized.x * strength;
    obj->speedY += normalized.y * strength;
}

bool IsGrounded(GameObject* obj, float checkDistance) {
    if (!obj) return false;
    
    const auto& allObjects = GameObject::GetAllObjects();
    for (GameObject* other : allObjects) {
        if (other == obj || !other->solid) continue;
        
        float otherTop = other->y - other->height / 2;
        float objBottom = obj->y + obj->height / 2;
        
        if (otherTop >= objBottom && otherTop <= objBottom + checkDistance) {
            if (obj->x + obj->width / 2 > other->x - other->width / 2 &&
                obj->x - obj->width / 2 < other->x + other->width / 2) {
                return true;
            }
        }
    }
    
    return false;
}

GameObject* Raycast(const Vector2& origin, const Vector2& direction, float maxDistance) {
    Vector2 normalized = direction.Normalized();
    
    const auto& allObjects = GameObject::GetAllObjects();
    GameObject* closestHit = nullptr;
    float closestDistance = maxDistance;
    
    for (GameObject* obj : allObjects) {
        if (!obj->collision) continue;
        
        Vector2 objPos(obj->x, obj->y);
        float halfWidth = obj->width / 2;
        float halfHeight = obj->height / 2;
        
        Vector2 toObj = objPos - origin;
        float distance = toObj.Length();
        
        if (distance < closestDistance) {
            if (toObj.x >= -halfWidth && toObj.x <= halfWidth &&
                toObj.y >= -halfHeight && toObj.y <= halfHeight) {
                closestHit = obj;
                closestDistance = distance;
            }
        }
    }
    
    return closestHit;
}

bool CheckOverlap(GameObject* objA, GameObject* objB) {
    if (!objA || !objB) return false;
    
    float aLeft = objA->x - objA->width / 2;
    float aRight = objA->x + objA->width / 2;
    float aTop = objA->y - objA->height / 2;
    float aBottom = objA->y + objA->height / 2;
    
    float bLeft = objB->x - objB->width / 2;
    float bRight = objB->x + objB->width / 2;
    float bTop = objB->y - objB->height / 2;
    float bBottom = objB->y + objB->height / 2;
    
    return !(aRight < bLeft || aLeft > bRight || aBottom < bTop || aTop > bBottom);
}

float GetDistance(GameObject* objA, GameObject* objB) {
    if (!objA || !objB) return 0.0f;
    
    Vector2 posA(objA->x, objA->y);
    Vector2 posB(objB->x, objB->y);
    return (posB - posA).Length();
}

std::vector<GameObject*> GetObjectsInRadius(const Vector2& center, float radius) {
    // Use spatial grid for O(n) instead of O(n²)
    return g_SpatialGrid.QueryRadius(center.x, center.y, radius);
}

// ============================================================================
// GameObject State Helpers
// ============================================================================

void SetActive(GameObject* obj, bool active) {
    if (obj) obj->active = active;
}

void ToggleVisibility(GameObject* obj) {
    if (obj) obj->visible = !obj->visible;
}

void FlipHorizontal(GameObject* obj) {
    if (obj) obj->flipX = !obj->flipX;
}

void FlipVertical(GameObject* obj) {
    if (obj) obj->flipY = !obj->flipY;
}

void LookAt(GameObject* obj, const Vector2& target) {
    if (!obj) return;
    
    Vector2 objPos(obj->x, obj->y);
    Vector2 direction = target - objPos;
    obj->angle = std::atan2(direction.y, direction.x) * 180.0f / Math::Constants::PI;
}

Vector2 GetDirectionTo(GameObject* obj, const Vector2& target) {
    if (!obj) return Vector2::Zero();
    
    Vector2 objPos(obj->x, obj->y);
    return (target - objPos).Normalized();
}

// ============================================================================
// Animation Helpers
// ============================================================================

void FadeAlpha(GameObject* obj, float targetAlpha, float duration) {
    if (!obj) return;
    
    Animation anim;
    anim.object = obj;
    anim.type = Animation::FADE;
    anim.startValue = obj->alpha;
    anim.targetValue = targetAlpha;
    anim.duration = duration;
    anim.elapsed = 0.0f;
    
    g_Animations.push_back(anim);
}

void ScaleOverTime(GameObject* obj, float targetScale, float duration) {
    if (!obj) return;
    
    Animation anim;
    anim.object = obj;
    anim.type = Animation::SCALE;
    anim.startValue = 1.0f; // Assume current scale is 1
    anim.targetValue = targetScale;
    anim.duration = duration;
    anim.elapsed = 0.0f;
    
    g_Animations.push_back(anim);
}

void RotateOverTime(GameObject* obj, float targetAngle, float duration) {
    if (!obj) return;
    
    Animation anim;
    anim.object = obj;
    anim.type = Animation::ROTATE;
    anim.startValue = obj->angle;
    anim.targetValue = targetAngle;
    anim.duration = duration;
    anim.elapsed = 0.0f;
    
    g_Animations.push_back(anim);
}

void MoveToPosition(GameObject* obj, const Vector2& target, float duration, bool easeIn) {
    if (!obj) return;
    
    Animation anim;
    anim.object = obj;
    anim.type = Animation::MOVE;
    anim.startPos = Vector2(obj->x, obj->y);
    anim.targetPos = target;
    anim.duration = duration;
    anim.elapsed = 0.0f;
    anim.easeIn = easeIn;
    
    g_Animations.push_back(anim);
}

// ============================================================================
// Collision & Trigger Helpers
// ============================================================================

bool IsCollidingWithTag(GameObject* obj, const std::string& tag) {
    if (!obj) return false;
    
    auto taggedObjects = FindGameObjectsWithTag(tag);
    for (GameObject* other : taggedObjects) {
        if (CheckOverlap(obj, other)) {
            return true;
        }
    }
    
    return false;
}

std::vector<GameObject*> GetCollidingObjects(GameObject* obj) {
    std::vector<GameObject*> result;
    if (!obj) return result;
    
    // Use spatial grid to get nearby objects only
    auto nearby = g_SpatialGrid.QueryNearby(obj);
    for (GameObject* other : nearby) {
        if (other != obj && CheckOverlap(obj, other)) {
            result.push_back(other);
        }
    }
    
    return result;
}

void SetCollisionLayer(GameObject* obj, int layer) {
    if (obj) {
        // Store layer in userData or a custom field
        // For now, just log it
        SAGE_INFO("Set collision layer {} for {}", layer, obj->name);
    }
}

// ============================================================================
// Audio Helpers
// ============================================================================

void PlaySoundAtPosition(const std::string& soundName, const Vector2& position, float volume) {
    // Integrate with AudioSystem via ServiceLocator
    if (ServiceLocator::HasGlobalInstance()) {
        auto& locator = ServiceLocator::GetGlobalInstance();
        if (locator.HasAudioSystem()) {
            auto& audio = locator.GetAudioSystem();
            if (audio.IsInitialized()) {
                audio.PlaySFX3D(soundName, position.x, position.y, 0.0f, volume);
                return;
            }
        }
    }
    
    SAGE_WARN("PlaySoundAtPosition: AudioSystem not available - {} at ({}, {})", 
              soundName, position.x, position.y);
}

void PlayMusic(const std::string& musicName, bool loop, float volume) {
    // Integrate with AudioSystem via ServiceLocator
    if (ServiceLocator::HasGlobalInstance()) {
        auto& locator = ServiceLocator::GetGlobalInstance();
        if (locator.HasAudioSystem()) {
            auto& audio = locator.GetAudioSystem();
            if (audio.IsInitialized()) {
                audio.PlayBGM(musicName, volume, loop ? 0.0f : 0.5f);
                return;
            }
        }
    }
    
    SAGE_WARN("PlayMusic: AudioSystem not available - {}", musicName);
}

void StopAllSounds() {
    // Integrate with AudioSystem via ServiceLocator
    if (ServiceLocator::HasGlobalInstance()) {
        auto& locator = ServiceLocator::GetGlobalInstance();
        if (locator.HasAudioSystem()) {
            auto& audio = locator.GetAudioSystem();
            if (audio.IsInitialized()) {
                audio.StopAll();
                return;
            }
        }
    }
    
    SAGE_WARN("StopAllSounds: AudioSystem not available");
}

void SetMasterVolume(float volume) {
    // Integrate with AudioSystem via ServiceLocator
    if (ServiceLocator::HasGlobalInstance()) {
        auto& locator = ServiceLocator::GetGlobalInstance();
        if (locator.HasAudioSystem()) {
            auto& audio = locator.GetAudioSystem();
            if (audio.IsInitialized()) {
                audio.SetMasterVolume(Math::Clamp01(volume));
                return;
            }
        }
    }
    
    SAGE_WARN("SetMasterVolume: AudioSystem not available");
}

// ============================================================================
// Color & Visual Effects
// ============================================================================

Color LerpColor(const Color& colorA, const Color& colorB, float t) {
    t = Math::Clamp01(t);
    return Color{
        colorA.r + (colorB.r - colorA.r) * t,
        colorA.g + (colorB.g - colorA.g) * t,
        colorA.b + (colorB.b - colorA.b) * t,
        colorA.a + (colorB.a - colorA.a) * t
    };
}

void FlashColor(GameObject* obj, const Color& flashColor, float duration) {
    if (!obj) return;
    (void)flashColor;  // Suppress unused parameter warning
    (void)duration;    // Suppress unused parameter warning
    // NOTE: Color flash animation requires integration with animation system
    SAGE_INFO("FlashColor for {} duration={}", obj->name, duration);
}

void CreateParticleEffect(const Vector2& position, const std::string& particleType, int count) {
    // TODO: Integrate with particle system
    SAGE_INFO("CreateParticleEffect: {} at ({}, {}) count={}", particleType, position.x, position.y, count);
}

// ============================================================================
// Scene Management
// ============================================================================

namespace {
    std::string g_CurrentSceneName = "MainScene";
    bool g_IsPaused = false;
}

void LoadScene(const std::string& sceneName) {
    SAGE_INFO("Loading scene: {}", sceneName);
    g_CurrentSceneName = sceneName;
    // TODO: Implement scene loading
}

void ReloadScene() {
    LoadScene(g_CurrentSceneName);
}

std::string GetCurrentSceneName() {
    return g_CurrentSceneName;
}

void PauseGame() {
    g_IsPaused = true;
}

void ResumeGame() {
    g_IsPaused = false;
}

bool IsPaused() {
    return g_IsPaused;
}

void QuitGame() {
    if (g_Window) {
        glfwSetWindowShouldClose(g_Window, GLFW_TRUE);
    }
}

// ============================================================================
// String & Text Utilities
// ============================================================================

std::string ToString(int value) {
    return std::to_string(value);
}

std::string ToString(float value, int precision) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.*f", precision, value);
    return std::string(buffer);
}

int ParseInt(const std::string& str, int defaultValue) {
    try {
        return std::stoi(str);
    } catch (...) {
        return defaultValue;
    }
}

float ParseFloat(const std::string& str, float defaultValue) {
    try {
        return std::stof(str);
    } catch (...) {
        return defaultValue;
    }
}

// ============================================================================
// Save & Load Helpers (PlayerPrefs)
// ============================================================================

namespace {
    std::map<std::string, std::string> g_PlayerPrefs;
}

void SetInt(const std::string& key, int value) {
    g_PlayerPrefs[key] = std::to_string(value);
}

int GetInt(const std::string& key, int defaultValue) {
    auto it = g_PlayerPrefs.find(key);
    if (it != g_PlayerPrefs.end()) {
        return ParseInt(it->second, defaultValue);
    }
    return defaultValue;
}

void SetFloat(const std::string& key, float value) {
    g_PlayerPrefs[key] = std::to_string(value);
}

float GetFloat(const std::string& key, float defaultValue) {
    auto it = g_PlayerPrefs.find(key);
    if (it != g_PlayerPrefs.end()) {
        return ParseFloat(it->second, defaultValue);
    }
    return defaultValue;
}

void SetString(const std::string& key, const std::string& value) {
    g_PlayerPrefs[key] = value;
}

std::string GetString(const std::string& key, const std::string& defaultValue) {
    auto it = g_PlayerPrefs.find(key);
    if (it != g_PlayerPrefs.end()) {
        return it->second;
    }
    return defaultValue;
}

void DeleteKey(const std::string& key) {
    g_PlayerPrefs.erase(key);
}

bool HasKey(const std::string& key) {
    return g_PlayerPrefs.find(key) != g_PlayerPrefs.end();
}

// ============================================================================
// Camera Follow Helpers
// ============================================================================

void CameraFollowObject(Camera2D* camera, GameObject* target, float smoothSpeed, const Vector2& offset) {
    if (!camera || !target) return;
    
    Vector2 targetPos(target->x + offset.x, target->y + offset.y);
    CameraFollowTarget(camera, targetPos, smoothSpeed);
}

void CameraSnapToObject(Camera2D* camera, GameObject* target, const Vector2& offset) {
    if (!camera || !target) return;
    
    camera->SetPosition(target->x + offset.x, target->y + offset.y);
}

// ============================================================================
// Player Movement Helpers
// ============================================================================

void MovePlayer(GameObject* player, float speed, bool autoFlip) {
    if (!player) return;
    
    float horizontal = GetAxis("Horizontal");
    float vertical = GetAxis("Vertical");
    
    player->speedX = horizontal * speed;
    player->speedY = vertical * speed;
    
    if (autoFlip && horizontal != 0.0f) {
        player->flipX = (horizontal < 0.0f);
    }
}

void MovePlatformer(GameObject* player, float speed, float jumpForce, bool autoFlip) {
    if (!player) return;
    
    // Horizontal movement
    float horizontal = GetAxis("Horizontal");
    player->speedX = horizontal * speed;
    
    // Auto-flip sprite
    if (autoFlip && horizontal != 0.0f) {
        player->flipX = (horizontal < 0.0f);
    }
    
    // Jump on Space/Jump button
    if (GetButtonDown("Jump")) {
        PlayerJump(player, jumpForce);
    }
}

void MoveTopDown(GameObject* player, float speed, bool normalize) {
    if (!player) return;
    
    float horizontal = GetAxis("Horizontal");
    float vertical = GetAxis("Vertical");
    
    Vector2 movement(horizontal, vertical);
    
    // Normalize diagonal movement to prevent faster diagonal speed
    if (normalize && (horizontal != 0.0f && vertical != 0.0f)) {
        float length = std::sqrt(horizontal * horizontal + vertical * vertical);
        if (length > 0.0f) {
            movement.x /= length;
            movement.y /= length;
        }
    }
    
    player->speedX = movement.x * speed;
    player->speedY = movement.y * speed;
}

bool PlayerJump(GameObject* player, float force) {
    if (!player) return false;
    
    // Check if grounded
    if (!player->IsGrounded()) {
        return false;
    }
    
    // Use specified force or player's default jump strength
    float jumpForce = (force > 0.0f) ? force : player->jumpStrength;
    
    // Apply upward velocity
    player->speedY = -jumpForce;
    
    return true;
}

void PlayerDash(GameObject* player, const Vector2& direction, float distance, float duration) {
    if (!player) return;
    
    // Normalize direction
    Vector2 dir = direction;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.0f) {
        dir.x /= len;
        dir.y /= len;
    }
    
    // Calculate dash velocity
    float dashSpeed = distance / duration;
    
    // Apply dash velocity
    player->speedX = dir.x * dashSpeed;
    player->speedY = dir.y * dashSpeed;
    
    // TODO: Could add animation state, invincibility, trail effect here
}

void MoveTowardsMouse(GameObject* player, float speed, float stopDistance) {
    if (!player) return;
    
    // Get mouse world position
    Vector2 mousePos = GetMouseWorldPosition();
    Vector2 playerPos(player->x, player->y);
    
    // Calculate direction to mouse
    Vector2 direction(mousePos.x - playerPos.x, mousePos.y - playerPos.y);
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    
    // Stop if close enough
    if (distance < stopDistance) {
        player->speedX = 0.0f;
        player->speedY = 0.0f;
        return;
    }
    
    // Normalize and apply speed
    direction.x /= distance;
    direction.y /= distance;
    
    player->speedX = direction.x * speed;
    player->speedY = direction.y * speed;
}

} // namespace GameUtility
} // namespace SAGE
