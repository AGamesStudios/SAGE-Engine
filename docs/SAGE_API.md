# SAGE API Overview

SAGE Engine exposes a very small set of commands so that most tasks can be done without writing code. The API mirrors what the editor offers.

## Scene access

```csharp
var scene = SageAPI.Scene; // currently loaded scene
```

## Creating an entity

```csharp
var cube = scene.CreateEntity("CubePrimitive");
```

## Adding components

```csharp
cube.AddComponent<SageMeshRenderer>();
cube.AddComponent<SageCollider>();
```

## Loading scenes

```csharp
SageAPI.LoadScene("MenuScene");
```

Every API call corresponds to a drag-and-drop operation or a button in the editor. The goal is to keep the scripting surface minimal.

