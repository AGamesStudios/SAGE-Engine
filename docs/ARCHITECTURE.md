# SAGE Engine Architecture

SAGE Engine builds on top of the Unity runtime but hides much of its complexity. The goal is to give creators a lightweight editor with a drag‑and‑drop workflow. The following sections outline the key modules and how they interact.

## Core Modules

1. **Engine Core** – wraps the Unity engine and provides project templates with sane defaults.
2. **Scene Manager** – loads and unloads scenes defined in simple YAML or JSON files.
3. **Entity Component System (ECS)** – exposes a minimal set of components (Transform, MeshRenderer, Collider, AudioSource and so on). Users build entities by checking components in the editor.
4. **Visual Scripting** – node graphs that rely on Unity's graph view. Only common actions and events are surfaced.
5. **Asset Pipeline** – drag‑and‑drop import of models, textures and audio with automatic conversion to Unity formats.
6. **UI Toolkit** – basic widgets (buttons, sliders, text) and a simple layout system. Meant for quick UI assembly.
7. **Input & Interaction** – default mappings for keyboard, mouse and gamepad. Actions are connected through the visual scripting graph.

## Editor Workflow

- **Project Setup** – SAGE creates a Unity project with a predefined folder layout.
- **Scene Editing** – scenes appear in a hierarchy view; users drag prefabs or primitives into the scene.
- **Component Configuration** – a minimal inspector with high‑level options; advanced Unity fields remain hidden.
- **Visual Scripting Graphs** – attach graphs to entities or scenes. Templates cover frequent tasks like playing sounds or switching levels.
- **Asset Browser** – imported models, textures and sounds appear in a simple browser for easy reuse.

## SAGE API

Although the focus is on visual editing, a small API is available for advanced users or automated scripts. The [SAGE API overview](SAGE_API.md) provides more snippets. Example (pseudo‑C#):

```csharp
// Access the current scene
var scene = SageAPI.Scene;

// Create an entity with a built-in prefab
var player = scene.CreateEntity("PlayerPrefab");

// Attach a script graph or enable built-in behaviors
player.AddComponent<SageCharacterController>();

// Load another scene
SageAPI.LoadScene("MainMenu");
```

This API mirrors the visual editor functionality. Each call has a corresponding UI action, ensuring consistency between no-code and code workflows.

## Extending

Developers familiar with Unity can access the full power of the engine if needed. Custom scripts or shaders can be added and then exposed to the SAGE editor by defining new components or visual nodes.


