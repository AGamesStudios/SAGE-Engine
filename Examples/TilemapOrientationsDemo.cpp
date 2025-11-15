#include "SAGE.h"
#include "ECS/Components/TilemapComponent.h"
#include "Graphics/Core/Rendering/TilemapRenderer.h"
#include "Graphics/Core/Camera2D.h"
#include "Core/ResourceManager.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

using namespace SAGE;
using namespace SAGE::ECS;

namespace {
    Camera2D camera;
    int currentOrientation = 0; // 0=Orthogonal, 1=Isometric, 2=Staggered, 3=Hexagonal
    const char* orientationNames[] = {"Orthogonal", "Isometric", "Staggered Y-Axis", "Hexagonal X-Axis"};
    
    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_ESCAPE) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            else if (key == GLFW_KEY_SPACE) {
                currentOrientation = (currentOrientation + 1) % 4;
                std::cout << "Switched to: " << orientationNames[currentOrientation] << std::endl;
            }
            else if (key == GLFW_KEY_R) {
                camera.position = {0.0f, 0.0f};
                camera.zoom = 1.0f;
                std::cout << "Reset camera" << std::endl;
            }
        }
    }
    
    TilemapComponent CreateOrthogonalTilemap() {
        TilemapComponent tilemap;
        tilemap.mapWidth = 15;
        tilemap.mapHeight = 12;
        tilemap.tileWidth = 32;
        tilemap.tileHeight = 32;
        tilemap.orientation = TilemapOrientation::Orthogonal;
        tilemap.renderOrder = TilemapRenderOrder::RightDown;
        
        // Simple tileset
        TilesetInfo tileset;
        tileset.name = "ortho_tiles";
        tileset.firstGID = 1;
        tileset.tileCount = 4;
        tileset.columns = 2;
        tileset.tileWidth = 32;
        tileset.tileHeight = 32;
        tilemap.tilesets.push_back(tileset);
        
        // Create layer with checkerboard pattern
        TilemapLayer layer;
        layer.name = "ground";
        layer.width = tilemap.mapWidth;
        layer.height = tilemap.mapHeight;
        layer.visible = true;
        layer.opacity = 1.0f;
        layer.tiles.resize(layer.width * layer.height);
        
        for (int y = 0; y < layer.height; ++y) {
            for (int x = 0; x < layer.width; ++x) {
                int idx = y * layer.width + x;
                // Create a pattern
                if (x == 0 || x == layer.width - 1 || y == 0 || y == layer.height - 1) {
                    layer.tiles[idx] = 2; // Border
                } else {
                    layer.tiles[idx] = ((x + y) % 2 == 0) ? 1 : 3;
                }
            }
        }
        
        tilemap.layers.push_back(layer);
        
        std::cout << "Created Orthogonal tilemap: " << tilemap.mapWidth << "x" << tilemap.mapHeight << std::endl;
        return tilemap;
    }
    
    TilemapComponent CreateIsometricTilemap() {
        TilemapComponent tilemap;
        tilemap.mapWidth = 12;
        tilemap.mapHeight = 12;
        tilemap.tileWidth = 64;
        tilemap.tileHeight = 32;
        tilemap.orientation = TilemapOrientation::Isometric;
        tilemap.renderOrder = TilemapRenderOrder::RightDown;
        
        TilesetInfo tileset;
        tileset.name = "iso_tiles";
        tileset.firstGID = 1;
        tileset.tileCount = 4;
        tileset.columns = 2;
        tileset.tileWidth = 64;
        tileset.tileHeight = 32;
        tilemap.tilesets.push_back(tileset);
        
        TilemapLayer layer;
        layer.name = "iso_ground";
        layer.width = tilemap.mapWidth;
        layer.height = tilemap.mapHeight;
        layer.visible = true;
        layer.opacity = 1.0f;
        layer.tiles.resize(layer.width * layer.height);
        
        for (int y = 0; y < layer.height; ++y) {
            for (int x = 0; x < layer.width; ++x) {
                int idx = y * layer.width + x;
                // Diamond pattern for isometric
                int dist = abs(x - layer.width/2) + abs(y - layer.height/2);
                if (dist < 3) {
                    layer.tiles[idx] = 1; // Center
                } else if (dist < 6) {
                    layer.tiles[idx] = 2; // Middle ring
                } else {
                    layer.tiles[idx] = 3; // Outer
                }
            }
        }
        
        tilemap.layers.push_back(layer);
        
        std::cout << "Created Isometric tilemap: " << tilemap.mapWidth << "x" << tilemap.mapHeight << std::endl;
        return tilemap;
    }
    
    TilemapComponent CreateStaggeredTilemap() {
        TilemapComponent tilemap;
        tilemap.mapWidth = 16;
        tilemap.mapHeight = 12;
        tilemap.tileWidth = 32;
        tilemap.tileHeight = 32;
        tilemap.orientation = TilemapOrientation::Staggered;
        tilemap.staggerAxis = TilemapStaggerAxis::Y;
        tilemap.staggerIndex = TilemapStaggerIndex::Odd;
        tilemap.renderOrder = TilemapRenderOrder::RightDown;
        
        TilesetInfo tileset;
        tileset.name = "stagger_tiles";
        tileset.firstGID = 1;
        tileset.tileCount = 4;
        tileset.columns = 2;
        tileset.tileWidth = 32;
        tileset.tileHeight = 32;
        tilemap.tilesets.push_back(tileset);
        
        TilemapLayer layer;
        layer.name = "stagger_ground";
        layer.width = tilemap.mapWidth;
        layer.height = tilemap.mapHeight;
        layer.visible = true;
        layer.opacity = 1.0f;
        layer.tiles.resize(layer.width * layer.height);
        
        for (int y = 0; y < layer.height; ++y) {
            for (int x = 0; x < layer.width; ++x) {
                int idx = y * layer.width + x;
                // Horizontal stripes that show stagger effect
                if (y % 4 == 0) {
                    layer.tiles[idx] = 1;
                } else if (y % 4 == 1) {
                    layer.tiles[idx] = 2;
                } else if (y % 4 == 2) {
                    layer.tiles[idx] = 3;
                } else {
                    layer.tiles[idx] = 1;
                }
            }
        }
        
        tilemap.layers.push_back(layer);
        
        std::cout << "Created Staggered tilemap (Y-axis, Odd): " << tilemap.mapWidth << "x" << tilemap.mapHeight << std::endl;
        return tilemap;
    }
    
    TilemapComponent CreateHexagonalTilemap() {
        TilemapComponent tilemap;
        tilemap.mapWidth = 14;
        tilemap.mapHeight = 10;
        tilemap.tileWidth = 28;
        tilemap.tileHeight = 32;
        tilemap.orientation = TilemapOrientation::Hexagonal;
        tilemap.staggerAxis = TilemapStaggerAxis::X;
        tilemap.staggerIndex = TilemapStaggerIndex::Even;
        tilemap.hexSideLength = 14;
        tilemap.renderOrder = TilemapRenderOrder::RightDown;
        
        TilesetInfo tileset;
        tileset.name = "hex_tiles";
        tileset.firstGID = 1;
        tileset.tileCount = 4;
        tileset.columns = 2;
        tileset.tileWidth = 28;
        tileset.tileHeight = 32;
        tilemap.tilesets.push_back(tileset);
        
        TilemapLayer layer;
        layer.name = "hex_ground";
        layer.width = tilemap.mapWidth;
        layer.height = tilemap.mapHeight;
        layer.visible = true;
        layer.opacity = 1.0f;
        layer.tiles.resize(layer.width * layer.height);
        
        for (int y = 0; y < layer.height; ++y) {
            for (int x = 0; x < layer.width; ++x) {
                int idx = y * layer.width + x;
                // Vertical stripes to show hex pattern
                if (x % 3 == 0) {
                    layer.tiles[idx] = 1;
                } else if (x % 3 == 1) {
                    layer.tiles[idx] = 2;
                } else {
                    layer.tiles[idx] = 3;
                }
            }
        }
        
        tilemap.layers.push_back(layer);
        
        std::cout << "Created Hexagonal tilemap (X-axis, Even, side=" << tilemap.hexSideLength << "): " 
                  << tilemap.mapWidth << "x" << tilemap.mapHeight << std::endl;
        return tilemap;
    }
}

int main() {
    std::cout << "=== SAGE Tilemap Orientations Demo ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  SPACE - Switch orientation" << std::endl;
    std::cout << "  R     - Reset camera" << std::endl;
    std::cout << "  WASD  - Move camera" << std::endl;
    std::cout << "  Q/E   - Zoom in/out" << std::endl;
    std::cout << "  ESC   - Exit" << std::endl;
    std::cout << std::endl;
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(1280, 720, "SAGE Tilemap Orientations Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);
    
    // Load OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // Setup camera
    camera.position = {0.0f, 0.0f};
    camera.zoom = 1.0f;
    camera.viewportWidth = 1280.0f;
    camera.viewportHeight = 720.0f;
    
    // Create all tilemaps
    std::cout << "\n--- Creating Tilemaps ---\n" << std::endl;
    TilemapComponent orthogonal = CreateOrthogonalTilemap();
    TilemapComponent isometric = CreateIsometricTilemap();
    TilemapComponent staggered = CreateStaggeredTilemap();
    TilemapComponent hexagonal = CreateHexagonalTilemap();
    
    std::cout << "\nAll tilemaps created successfully!" << std::endl;
    std::cout << "Starting with: " << orientationNames[currentOrientation] << std::endl;
    std::cout << "\nPress SPACE to switch between orientations..." << std::endl;
    
    // Main loop
    float lastTime = static_cast<float>(glfwGetTime());
    
    while (!glfwWindowShouldClose(window)) {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        // Input
        glfwPollEvents();
        
        // Camera controls
        const float cameraSpeed = 300.0f * deltaTime;
        const float zoomSpeed = 2.0f * deltaTime;
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.position.y -= cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.position.y += cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.position.x -= cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.position.x += cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camera.zoom = std::min(5.0f, camera.zoom + zoomSpeed);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camera.zoom = std::max(0.1f, camera.zoom - zoomSpeed);
        
        // Update viewport
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        camera.viewportWidth = static_cast<float>(width);
        camera.viewportHeight = static_cast<float>(height);
        
        // Clear
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, width, height);
        
        // Render current tilemap
        Float2 position = {100.0f, 100.0f};
        
        switch (currentOrientation) {
            case 0:
                if (orthogonal.IsValid()) {
                    TilemapRenderer::Render(orthogonal, position, camera);
                }
                break;
            case 1:
                if (isometric.IsValid()) {
                    TilemapRenderer::Render(isometric, position, camera);
                }
                break;
            case 2:
                if (staggered.IsValid()) {
                    TilemapRenderer::Render(staggered, position, camera);
                }
                break;
            case 3:
                if (hexagonal.IsValid()) {
                    TilemapRenderer::Render(hexagonal, position, camera);
                }
                break;
        }
        
        glfwSwapBuffers(window);
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    std::cout << "\nDemo closed." << std::endl;
    return 0;
}
