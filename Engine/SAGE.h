#pragma once

// Главный заголовочный файл SAGE Engine

// Core
#include "Core/Core.h"
#include "Core/Application.h"
#include "Core/Window.h"
#include "Core/WindowProps.h"
#include "Core/Logger.h"
#include "Core/FileLogger.h"
#include "Core/CrashHandler.h"
#include "Core/Version.h"
#include "Core/Profiler.h"
#include "Core/GameObject.h"
#include "Core/Event.h"
#include "Core/EventBus.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/WindowEvent.h"
#include "Core/Events/InputEvent.h"
#include "Core/Scene.h"
#include "Core/SceneStack.h"
#include "Core/SceneState.h"
#include "Core/SceneManager.h"
#include "Core/ServiceLocator.h"
#include "Core/ResourceManager.h"

// Graphics
#include "Graphics/API/Renderer.h"
#include "Graphics/Core/Resources/Shader.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Graphics/Core/Resources/Sprite.h"
#include "Graphics/Core/Resources/Font.h"
#include "Graphics/Rendering/Effects/Particles/ParticleSystem.h"

// Input System (Action-Based)
#include "Input/KeyCodes.h"
#include "Input/InputAction.h"
#include "Input/InputMap.h"
#include "Input/ActionContext.h"
#include "Input/InputBuffer.h"

// Math
#include "Math/Math.h"
#include "Math/Vector2.h"
#include "Math/Random.h"

// Physics
#include "Physics/PhysicsContact.h"
// PhysicsWorld.h removed - use ECS PhysicsSystem instead

// Resources
#include "Resources/ResourceRegistry.h"

