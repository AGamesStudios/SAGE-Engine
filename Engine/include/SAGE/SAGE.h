#pragma once

// ============================================
// SAGE Engine - Alpha v0.1.0
// Simple And Game Engine for 2D Games
// ============================================

// Core Systems
#include "SAGE/Application.h"
#include "SAGE/ApplicationConfig.h"
#include "SAGE/DevMode.h"
#include "SAGE/Log.h"
#include "SAGE/Logger.h"
#include "SAGE/Time.h"
#include "SAGE/Window.h"
#include "SAGE/WindowConfig.h"

// Math
#include "SAGE/Math/Vector2.h"
#include "SAGE/Math/Color.h"
#include "SAGE/Math/Matrix3.h"
#include "SAGE/Math/Rect.h"

// Input
#include "SAGE/Input/Input.h"

// Graphics
#include "SAGE/Graphics/Renderer.h"
#include "SAGE/Graphics/Shader.h"
#include "SAGE/Graphics/ShaderLibrary.h"
#include "SAGE/Graphics/Texture.h"
#include "SAGE/Graphics/Camera2D.h"
#include "SAGE/Graphics/Sprite.h"
#include "SAGE/Graphics/Animation.h"
#include "SAGE/Graphics/ParticleSystem.h"
#include "SAGE/Graphics/ParticleEmitter.h"
#include "SAGE/Graphics/Font.h"
#include "SAGE/Graphics/Tilemap.h"

// Audio
#include "SAGE/Audio/Audio.h"

// Core Game Systems
#include "SAGE/Core/ResourceManager.h"
#include "SAGE/Core/Scene.h"
#include "SAGE/Core/SceneManager.h"
#include "SAGE/Core/Game.h"
#include "SAGE/Core/ECS.h"
#include "SAGE/Core/ECSComponents.h"
#include "SAGE/Core/ECSSystems.h"
#include "SAGE/Core/ECSGame.h"
#include "SAGE/Graphics/Gizmo.h"

// Plugin System
#include "SAGE/Plugin/IPlugin.h"
#include "SAGE/Plugin/PluginManager.h"
