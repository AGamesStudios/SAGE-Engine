#pragma once

#include "GameObject.h"
#include "Scene.h"
#include "../Math/Vector2.h"
#include "../Math/Constants.h"
#include <string>
#include <vector>
#include <random>

namespace SAGE {

// Forward declarations
class Camera2D;

/**
 * @brief Utility functions for game development
 * 
 * Provides convenient helper methods for common game development tasks:
 * - GameObject management (Find, Instantiate)
 * - Scene operations
 * - Input queries
 * - Math utilities
 * - Time management
 */
namespace GameUtility {

    // ============================================================================
    // GameObject Management
    // ============================================================================

    /**
     * @brief Create a copy of GameObject at runtime
     * @param original GameObject to copy
     * @param position Position for new instance
     * @return Pointer to new GameObject
     */
    GameObject* Instantiate(GameObject* original, const Vector2& position = Vector2::Zero());

    /**
     * @brief Find all GameObjects with specific tag
     * @param tag Tag to search for
     * @return Vector of matching GameObjects
     */
    std::vector<GameObject*> FindGameObjectsWithTag(const std::string& tag);

    /**
     * @brief Find first GameObject with specific name
     * @param name Name to search for
     * @return Pointer to GameObject or nullptr
     */
    GameObject* FindGameObjectByName(const std::string& name);

    /**
     * @brief Find first GameObject with specific tag
     * @param tag Tag to search for
     * @return Pointer to GameObject or nullptr
     */
    GameObject* FindGameObjectWithTag(const std::string& tag);

    /**
     * @brief Destroy GameObject after delay
     * @param obj GameObject to destroy
     * @param delay Delay in seconds
     */
    void Destroy(GameObject* obj, float delay = 0.0f);

    // ============================================================================
    // Input Helpers
    // ============================================================================

    /**
     * @brief Get axis value (-1 to 1) for movement
     * @param axisName "Horizontal" or "Vertical"
     * @return Axis value
     * 
     * Horizontal: A/D or Left/Right arrows
     * Vertical: W/S or Up/Down arrows
     */
    float GetAxis(const std::string& axisName);

    /**
     * @brief Check if button is pressed this frame
     * @param buttonName Button name ("Jump", "Fire", "Interact")
     * @return True if pressed
     */
    bool GetButtonDown(const std::string& buttonName);

    /**
     * @brief Check if button is held
     * @param buttonName Button name
     * @return True if held
     */
    bool GetButton(const std::string& buttonName);

    /**
     * @brief Get mouse movement delta
     * @return Mouse delta vector
     */
    Vector2 GetMouseDelta();

    /**
     * @brief Get mouse position in world coordinates
     * @param camera Camera to use for conversion (nullptr = default)
     * @return World position
     */
    Vector2 GetMouseWorldPosition(Camera2D* camera = nullptr);

    // ============================================================================
    // Time Management
    // ============================================================================

    /**
     * @brief Get delta time for current frame
     * @return Delta time in seconds
     */
    float GetDeltaTime();

    /**
     * @brief Get time since game started
     * @return Time in seconds
     */
    float GetTimeSinceStartup();

    /**
     * @brief Get current frame count
     * @return Frame number
     */
    uint64_t GetFrameCount();

    /**
     * @brief Get fixed timestep for physics
     * @return Fixed delta time
     */
    float GetFixedDeltaTime();

    // ============================================================================
    // Math Utilities
    // ============================================================================

    /**
     * @brief Linear interpolation
     * @param a Start value
     * @param b End value
     * @param t Interpolation factor (0-1)
     * @return Interpolated value
     */
    template<typename T>
    T Lerp(T a, T b, float t) {
        return a + (b - a) * Math::Clamp01(t);
    }

    /**
     * @brief Smoothly interpolate towards target
     * @param current Current value
     * @param target Target value
     * @param currentVelocity Current velocity (modified)
     * @param smoothTime Smoothing time
     * @param maxSpeed Maximum speed
     * @param deltaTime Delta time
     * @return New value
     */
    float SmoothDamp(float current, float target, float& currentVelocity, 
                     float smoothTime, float maxSpeed = INFINITY, float deltaTime = -1.0f);

    /**
     * @brief Vector2 smooth damp
     */
    Vector2 SmoothDamp(const Vector2& current, const Vector2& target, 
                       Vector2& currentVelocity, float smoothTime, 
                       float maxSpeed = INFINITY, float deltaTime = -1.0f);

    /**
     * @brief Move towards target with max distance delta
     * @param current Current value
     * @param target Target value
     * @param maxDelta Maximum movement per frame
     * @return New value
     */
    float MoveTowards(float current, float target, float maxDelta);

    /**
     * @brief Vector2 move towards
     */
    Vector2 MoveTowards(const Vector2& current, const Vector2& target, float maxDelta);

    // ============================================================================
    // Random Utilities
    // ============================================================================

    namespace Random {
        /**
         * @brief Random float in range [min, max]
         */
        float Range(float min, float max);

        /**
         * @brief Random int in range [min, max] (inclusive)
         */
        int Range(int min, int max);

        /**
         * @brief Random float in range [0, 1]
         */
        float Value();

        /**
         * @brief Random point in circle
         */
        Vector2 InsideUnitCircle();

        /**
         * @brief Random point on circle edge
         */
        Vector2 OnUnitCircle();

        /**
         * @brief Set random seed
         */
        void SetSeed(unsigned int seed);
    }

    // ============================================================================
    // Screen & Display
    // ============================================================================

    /**
     * @brief Get screen width
     */
    int GetScreenWidth();

    /**
     * @brief Get screen height
     */
    int GetScreenHeight();

    /**
     * @brief Set fullscreen mode
     */
    void SetFullscreen(bool fullscreen);

    /**
     * @brief Check if fullscreen
     */
    bool IsFullscreen();

    /**
     * @brief Set VSync
     */
    void SetVSync(bool enabled);

    // ============================================================================
    // Camera Utilities
    // ============================================================================

    /**
     * @brief Shake camera with intensity and duration
     * @param camera Camera to shake
     * @param intensity Shake intensity
     * @param duration Duration in seconds
     */
    void ShakeCamera(Camera2D* camera, float intensity, float duration);

    /**
     * @brief Smoothly follow target
     * @param camera Camera to move
     * @param target Target position
     * @param smoothSpeed Smoothing factor (0-1)
     */
    void CameraFollowTarget(Camera2D* camera, const Vector2& target, float smoothSpeed = 0.125f);

    /**
     * @brief Set camera bounds
     * @param camera Camera to constrain
     * @param minX Minimum X
     * @param minY Minimum Y
     * @param maxX Maximum X
     * @param maxY Maximum Y
     */
    void SetCameraBounds(Camera2D* camera, float minX, float minY, float maxX, float maxY);

    /**
     * @brief Make camera follow GameObject smoothly
     * @param camera Camera to control
     * @param target GameObject to follow
     * @param smoothSpeed Smoothing factor (0-1, default 0.125)
     * @param offset Camera offset from target
     */
    void CameraFollowObject(Camera2D* camera, GameObject* target, float smoothSpeed = 0.125f, const Vector2& offset = Vector2::Zero());

    /**
     * @brief Make camera instantly snap to GameObject position
     * @param camera Camera to control
     * @param target GameObject to follow
     * @param offset Camera offset from target
     */
    void CameraSnapToObject(Camera2D* camera, GameObject* target, const Vector2& offset = Vector2::Zero());

    // ============================================================================
    // Physics Utilities
    // ============================================================================

    /**
     * @brief Apply impulse force to object
     * @param obj Target object
     * @param force Force vector
     */
    void ApplyImpulse(GameObject* obj, const Vector2& force);

    /**
     * @brief Apply force in direction
     * @param obj Target object
     * @param direction Direction vector
     * @param strength Force strength
     */
    void ApplyForce(GameObject* obj, const Vector2& direction, float strength);

    /**
     * @brief Check if object is grounded
     * @param obj Object to check
     * @param checkDistance Distance to check below
     * @return True if grounded
     */
    bool IsGrounded(GameObject* obj, float checkDistance = 2.0f);

    /**
     * @brief Raycast from point in direction
     * @param origin Ray origin
     * @param direction Ray direction
     * @param maxDistance Maximum distance
     * @return Hit object or nullptr
     */
    GameObject* Raycast(const Vector2& origin, const Vector2& direction, float maxDistance = 1000.0f);

    /**
     * @brief Check overlap between two objects
     * @param objA First object
     * @param objB Second object
     * @return True if overlapping
     */
    bool CheckOverlap(GameObject* objA, GameObject* objB);

    /**
     * @brief Get distance between two objects
     * @param objA First object
     * @param objB Second object
     * @return Distance
     */
    float GetDistance(GameObject* objA, GameObject* objB);

    /**
     * @brief Get all objects within radius
     * @param center Center position
     * @param radius Search radius
     * @return Vector of objects
     */
    std::vector<GameObject*> GetObjectsInRadius(const Vector2& center, float radius);

    // ============================================================================
    // Player Movement Helpers
    // ============================================================================

    /**
     * @brief Simple horizontal movement with WASD/Arrows
     * @param player Player object to move
     * @param speed Movement speed in pixels/second
     * @param autoFlip Auto-flip sprite based on direction
     * 
     * Example:
     *   MovePlayer(player, 200.0f, true);
     */
    void MovePlayer(GameObject* player, float speed, bool autoFlip = true);

    /**
     * @brief Platformer-style movement (left/right + jump)
     * @param player Player object
     * @param speed Horizontal speed
     * @param jumpForce Jump force (default 650)
     * @param autoFlip Auto-flip sprite
     * 
     * Example:
     *   MovePlatformer(player, 250.0f, 650.0f, true);
     */
    void MovePlatformer(GameObject* player, float speed, float jumpForce = 650.0f, bool autoFlip = true);

    /**
     * @brief Top-down 8-direction movement
     * @param player Player object
     * @param speed Movement speed
     * @param normalize Normalize diagonal movement
     * 
     * Example:
     *   MoveTopDown(player, 200.0f, true);
     */
    void MoveTopDown(GameObject* player, float speed, bool normalize = true);

    /**
     * @brief Make player jump (if grounded)
     * @param player Player object
     * @param force Jump force (default uses player's jumpStrength)
     * @return True if jump was executed
     * 
     * Example:
     *   if (Input::GetKeyDown(KeyCode::Space)) {
     *       PlayerJump(player, 700.0f);
     *   }
     */
    bool PlayerJump(GameObject* player, float force = 0.0f);

    /**
     * @brief Apply dash/dodge movement
     * @param player Player object
     * @param direction Direction to dash
     * @param distance Dash distance
     * @param duration Dash duration in seconds
     * 
     * Example:
     *   Vector2 dir = GetAxis("Horizontal") > 0 ? Vector2::Right() : Vector2::Left();
     *   PlayerDash(player, dir, 150.0f, 0.2f);
     */
    void PlayerDash(GameObject* player, const Vector2& direction, float distance, float duration);

    /**
     * @brief Simple mouse-follow movement
     * @param player Player object
     * @param speed Movement speed
     * @param stopDistance Minimum distance to mouse
     * 
     * Example:
     *   MoveTowardsMouse(player, 180.0f, 10.0f);
     */
    void MoveTowardsMouse(GameObject* player, float speed, float stopDistance = 5.0f);

    // ============================================================================
    // GameObject State Helpers
    // ============================================================================

    /**
     * @brief Set object active state
     * @param obj Target object
     * @param active Active state
     */
    void SetActive(GameObject* obj, bool active);

    /**
     * @brief Toggle object visibility
     * @param obj Target object
     */
    void ToggleVisibility(GameObject* obj);

    /**
     * @brief Flip object horizontally
     * @param obj Target object
     */
    void FlipHorizontal(GameObject* obj);

    /**
     * @brief Flip object vertically
     * @param obj Target object
     */
    void FlipVertical(GameObject* obj);

    /**
     * @brief Rotate object to look at point
     * @param obj Target object
     * @param target Point to look at
     */
    void LookAt(GameObject* obj, const Vector2& target);

    /**
     * @brief Get direction from object to target
     * @param obj Source object
     * @param target Target position
     * @return Normalized direction vector
     */
    Vector2 GetDirectionTo(GameObject* obj, const Vector2& target);

    // ============================================================================
    // Animation Helpers
    // ============================================================================

    /**
     * @brief Fade object alpha over time
     * @param obj Target object
     * @param targetAlpha Target alpha (0-1)
     * @param duration Duration in seconds
     */
    void FadeAlpha(GameObject* obj, float targetAlpha, float duration);

    /**
     * @brief Scale object over time
     * @param obj Target object
     * @param targetScale Target scale
     * @param duration Duration in seconds
     */
    void ScaleOverTime(GameObject* obj, float targetScale, float duration);

    /**
     * @brief Rotate object over time
     * @param obj Target object
     * @param targetAngle Target angle in degrees
     * @param duration Duration in seconds
     */
    void RotateOverTime(GameObject* obj, float targetAngle, float duration);

    /**
     * @brief Move object to position over time
     * @param obj Target object
     * @param target Target position
     * @param duration Duration in seconds
     * @param easeIn Use ease-in interpolation
     */
    void MoveToPosition(GameObject* obj, const Vector2& target, float duration, bool easeIn = false);

    // ============================================================================
    // Collision & Trigger Helpers
    // ============================================================================

    /**
     * @brief Check if object is colliding with tag
     * @param obj Object to check
     * @param tag Tag to check against
     * @return True if colliding
     */
    bool IsCollidingWithTag(GameObject* obj, const std::string& tag);

    /**
     * @brief Get all objects colliding with object
     * @param obj Object to check
     * @return Vector of colliding objects
     */
    std::vector<GameObject*> GetCollidingObjects(GameObject* obj);

    /**
     * @brief Set collision layer
     * @param obj Target object
     * @param layer Layer number
     */
    void SetCollisionLayer(GameObject* obj, int layer);

    // ============================================================================
    // Audio Helpers
    // ============================================================================

    /**
     * @brief Play sound at position
     * @param soundName Sound file name
     * @param position World position
     * @param volume Volume (0-1)
     */
    void PlaySoundAtPosition(const std::string& soundName, const Vector2& position, float volume = 1.0f);

    /**
     * @brief Play background music
     * @param musicName Music file name
     * @param loop Loop playback
     * @param volume Volume (0-1)
     */
    void PlayMusic(const std::string& musicName, bool loop = true, float volume = 0.5f);

    /**
     * @brief Stop all sounds
     */
    void StopAllSounds();

    /**
     * @brief Set master volume
     * @param volume Volume (0-1)
     */
    void SetMasterVolume(float volume);

    // ============================================================================
    // Color & Visual Effects
    // ============================================================================

    /**
     * @brief Interpolate between colors
     * @param colorA Start color
     * @param colorB End color
     * @param t Interpolation factor (0-1)
     * @return Interpolated color
     */
    struct Color { float r, g, b, a; };
    Color LerpColor(const Color& colorA, const Color& colorB, float t);

    /**
     * @brief Flash object color
     * @param obj Target object
     * @param flashColor Flash color
     * @param duration Duration in seconds
     */
    void FlashColor(GameObject* obj, const Color& flashColor, float duration);

    /**
     * @brief Create particle effect
     * @param position Spawn position
     * @param particleType Effect type
     * @param count Particle count
     */
    void CreateParticleEffect(const Vector2& position, const std::string& particleType, int count = 10);

    // ============================================================================
    // Scene Management
    // ============================================================================

    /**
     * @brief Load scene by name
     * @param sceneName Scene name
     */
    void LoadScene(const std::string& sceneName);

    /**
     * @brief Reload current scene
     */
    void ReloadScene();

    /**
     * @brief Get current scene name
     * @return Scene name
     */
    std::string GetCurrentSceneName();

    /**
     * @brief Pause game
     */
    void PauseGame();

    /**
     * @brief Resume game
     */
    void ResumeGame();

    /**
     * @brief Check if game is paused
     * @return True if paused
     */
    bool IsPaused();

    /**
     * @brief Quit application
     */
    void QuitGame();

    // ============================================================================
    // String & Text Utilities
    // ============================================================================

    /**
     * @brief Convert int to string
     * @param value Value to convert
     * @return String representation
     */
    std::string ToString(int value);

    /**
     * @brief Convert float to string with precision
     * @param value Value to convert
     * @param precision Decimal places
     * @return String representation
     */
    std::string ToString(float value, int precision = 2);

    /**
     * @brief Parse string to int
     * @param str String to parse
     * @param defaultValue Default if parse fails
     * @return Parsed value
     */
    int ParseInt(const std::string& str, int defaultValue = 0);

    /**
     * @brief Parse string to float
     * @param str String to parse
     * @param defaultValue Default if parse fails
     * @return Parsed value
     */
    float ParseFloat(const std::string& str, float defaultValue = 0.0f);

    // ============================================================================
    // Save & Load Helpers
    // ============================================================================

    /**
     * @brief Save int value to PlayerPrefs
     * @param key Key name
     * @param value Value to save
     */
    void SetInt(const std::string& key, int value);

    /**
     * @brief Get int value from PlayerPrefs
     * @param key Key name
     * @param defaultValue Default if not found
     * @return Saved value or default
     */
    int GetInt(const std::string& key, int defaultValue = 0);

    /**
     * @brief Save float value
     * @param key Key name
     * @param value Value to save
     */
    void SetFloat(const std::string& key, float value);

    /**
     * @brief Get float value
     * @param key Key name
     * @param defaultValue Default if not found
     * @return Saved value or default
     */
    float GetFloat(const std::string& key, float defaultValue = 0.0f);

    /**
     * @brief Save string value
     * @param key Key name
     * @param value Value to save
     */
    void SetString(const std::string& key, const std::string& value);

    /**
     * @brief Get string value
     * @param key Key name
     * @param defaultValue Default if not found
     * @return Saved value or default
     */
    std::string GetString(const std::string& key, const std::string& defaultValue = "");

    /**
     * @brief Delete saved value
     * @param key Key name
     */
    void DeleteKey(const std::string& key);

    /**
     * @brief Check if key exists
     * @param key Key name
     * @return True if exists
     */
    bool HasKey(const std::string& key);

} // namespace GameUtility

} // namespace SAGE
