# SAGE Engine

This repository contains **SAGE Engine**, a lightweight core framework for
games.  **SAGE 2D** extends the engine with sprite rendering while **SAGE
Editor** lets you place objects visually and save projects.  The editor remains
small so it runs well even on older computers.

The current release is **SAGE Engine: 2D prototype v0.0.01a**. The engine
stores its version inside every `.sageproject` file so you can safely upgrade
without losing progress.

## Architecture
The core engine code resides under `engine/core` which defines the
generic `GameObject`, `Scene`, `Engine` and `Project` classes. 2D helpers are
provided in the same package, and the event system lives in
`engine/logic`.  High level helpers live in `engine/api` so scripts can load,
save and run projects in just a few lines. **SAGE Editor** builds on these pieces
but remains optional so games can depend on the engine without pulling in the
editor.  Simply import the engine from `engine` and the editor from
`sage_editor` to keep them modular.  For convenience a lightweight ``sage``
package re-exports both modules so ``import sage.engine`` and ``import
sage.editor`` continue to work.  The ``sage_sdk`` package provides shared
utilities like the plugin loader used by both components.
The editor code is split into ``sage_editor.editor`` for the main window and
``sage_editor.app`` which contains the startup logic and project manager.
Dock widgets live under ``sage_editor.docks`` while reusable widgets like the
OpenGL viewport reside in ``sage_editor.widgets``. Embedding the editor in
other tools only requires importing these modules.

### Renderer

Rendering is handled by modules under `engine/renderers`.  The
engine ships with an `OpenGLRenderer` implemented using glfw and
PyOpenGL.  Renderers register themselves through a small registry
so projects can select a backend by name.  The base `Renderer`
interface allows additional backends (for example Vulkan) to be added
without touching the core engine.
Resources are loaded through `ResourceManager` which resolves paths
relative to the project's `resources` folder so assets remain
organized and portable.
`OpenGLRenderer` accepts a `GLSettings` object so projects can control
OpenGL context versions and toggle vsync if needed. The renderer relies on
**GLM** for its projection matrix so custom backends can produce compatible
matrices easily.
The editor's viewport also uses an OpenGL widget and calls `glViewport` on
resize so what you see while editing matches the game window exactly.
When launching the editor a **Project Manager** window appears. It lists your
recent projects with their creation date and full path.  Buttons let you create
a new project, open an existing file or clear the list for a clean start. The
dialog now groups the table under a "Recent Projects" heading with larger
fonts and spacing so paths are easy to read. Right click a project to open it
immediately or remove it from the list.
Once a project is chosen the editor opens maximized in a dark Fusion
theme and provides two
tabs: **Viewport** and **Logic**. The viewport uses a virtually infinite
`QGraphicsScene` so you can pan freely. It renders through a Qt
`QOpenGLWidget` so what you see matches the OpenGL runtime. Hold **Ctrl** and
scroll to zoom the view. A configurable grid and axis lines can be toggled from
the toolbar. Grid size and color are adjustable and snapping ensures objects
align cleanly when enabled.
An **Add Object** button beneath the object list places a white square sprite
with a default name like `New Object`. Double-click an object to open a
small editor for changing its image or RGBA color. Clicking a sprite selects it
and the cursor switches between an open
and closed hand while dragging. A yellow gizmo outline appears with a corner
handle for scaling and a small crosshair above for rotation so objects are easy
to manipulate. The gizmo and handles always appear above other sprites so they
remain visible. Use the **Show Gizmo** toolbar button to hide or show them as
needed. A cyan rectangle indicates the active camera frustum. If no camera
exists the window size is used instead. The active camera automatically
updates its width and height to match the window whenever it is resized,
preserving the aspect ratio. Camera objects now include a **Z**
position so they can be layered with sprites. Projects are saved in a single
`.sageproject` file
All objects are listed in a dock on the right. Selecting one shows a **Transform**
panel with X, Y, Z, separate Scale X/Y and Rotation fields. A *Link XY* checkbox
lets you keep both scales in sync. Rotation now accounts for non-uniform
scaling so objects spin correctly even when Scale X and Scale Y differ. Scaling
also stays centered on the sprite regardless of rotation. Transform calculations
now use **GLM** so scaling occurs before rotation and objects rotate around
their center without skewing. Each object also defines a **pivot** so the
viewport matches the runtime and rotations occur around the desired point. The transform dock also includes a **Coordinate Mode** drop-down for switching
between *Global* and *Local* coordinates. In local mode the gizmo rotates with
the object so scaling and rotating follow its orientation. Internally every
object stores its rotation as a quaternion so angles remain stable even after
many incremental edits. Each object also keeps its own settings dictionary so
properties remain independent across different items. The
project file stores the entire scene
including object positions, events and variables.
A **Resources** dock on the left lists everything under your project's
`resources` folder. Buttons above the tree and a right click menu let you create
folders anywhere in the hierarchy or import new files. A search field filters
the view and you can drag items between folders. Files are sorted alphabetically
so projects stay organized. Any image chosen from outside the project is copied
into this directory automatically, so scenes never reference files that might
disappear. The engine loads all assets relative to this directory, ensuring
reorganizing files will not break existing scenes.
If PyQt does not provide ``QFileSystemModel`` the editor falls back to a
simpler tree widget that still lets you create folders and import files.
Use **File → New Project** to generate a folder for your game. The dialog asks
for a project name and location; the engine currently uses the OpenGL renderer.
It then creates the folder with a `.sageproject` file. Each new object
receives a generic name like `New Object (1)` so conditions always target the
correct item. The editor disables sprite, variable and logic actions until a
project is opened, ensuring all changes are saved to a `.sageproject` file.
The **Logic** tab lists object events with
conditions on the left and actions on the right. The event dialog now provides
drop-down lists for keys and shows only parameters relevant to the chosen
action. Variables of type int, float, string and bool can be defined and used
in conditions or actions. Mathematical actions only apply to int or float
variables, ensuring booleans and strings remain unchanged. When setting a variable in an action the name is
chosen from a drop-down list and booleans use a check box. Events attach to specific objects and can trigger on
game start or every frame. Fonts are slightly larger for readability. Use the
*Run* button in the toolbar to launch the current scene directly from the
editor. Temporary files are cleaned up each time so you can run repeatedly
without crashes.
Window dimensions can be changed under **Settings → Window Settings**. The
game window title matches the editor, e.g. `SAGE Editor: MyGame - Scene1`.
When you edit the scene the title gains an `(unsaved)` suffix until you save.
Project files now record the window `width`, `height` and `title` so the engine
launches with exactly the same viewport size. Future versions may add other
renderer backends, and each project remembers which renderer to use.
When defining variables, boolean values are edited with a convenient check box
instead of typing "true" or "false".
When comparing variables, the name is selected from a drop-down list so typos
are avoided.

Sprites are loaded lazily at runtime so the editor no longer relies on
any particular window system. This prevents crashes when adding images on systems
without an SDL window. The editor also validates image and variable input and
ensures combo boxes always point to valid objects, preventing crashes when
adding multiple sprites, variables, or conditions on older hardware.
Any errors when creating conditions, actions, or variables are caught and
printed to the console instead of closing the editor.
If something goes wrong during gameplay, the console shows the full
Python traceback so you can identify exactly where the problem occurred.
Use the **Clear Log** toolbar button to empty the console at any time.

### Localization

The editor supports multiple languages. On startup it selects the language
matching your system locale, but you can change it via the toolbar drop-down.
All strings come from simple dictionaries in `sage_editor/lang.py`, so adding
more languages only requires editing this file. Translations cover file dialogs,
logic windows and event lists so the whole interface is localized.
Old projects saved with translated condition or action names still load
correctly because the engine maps them back to their English identifiers at
runtime.  The mapping comes from simple dictionaries included with both the
editor and the engine (`sage_editor/lang.py` and `engine/lang.py`), so even
projects opened without the editor present continue to load.
The loader always converts localized condition and action names back to their
English identifiers before resolving them, so translated files remain
compatible with future releases.

The condition and action lists offer context menus with **Edit**, **Copy**,
**Paste**, and **Delete** options. Right-click an empty area to add a new block
or paste the previously copied one.

Run a saved project with:

```bash
python -m engine path/to/project.sageproject
# the engine uses the renderer stored in the project file
# pass `--renderer opengl` or any registered name to override
```

Project files store the entire scene data so you can share a single file. Use
**File → Save Project** to write the current project. The **Recent Projects**
submenu lists the last few files you opened for quick access.
If you make changes after saving, the window title shows `(unsaved)` so you
know there are modifications. Closing the editor with unsaved work displays a
dialog asking whether to save before exiting.

Launch the editor with:

```bash
python main.py
# or
python -m sage_editor
# or from Python
python -c "import sage_editor as ed; ed.main()"
```

Sprite positions are stored when you save so the runtime engine can render them
exactly as placed in the editor.
Event definitions are saved as well, so running the scene will include any logic
you created in the editor.

### SAGE Logic Events

The engine ships with **SAGE Logic**, a small condition/action system inspired
by Clickteam. Events consist of *conditions* and *actions*. When all conditions
pass, the actions run. Built-in blocks include `KeyPressed`, `Collision`,
`AfterTime`, `Move`, `SetPosition`, `Destroy` and `Print`. `AfterTime` accepts
hours, minutes and seconds so you can delay events precisely. You can subclass
`Condition` or `Action` to create your own.  Conditions and actions live in
separate modules and register themselves automatically so new types can be
added without touching the core loader.  The `logic` package automatically
imports all of its submodules on startup so any plugin that registers new
conditions or actions becomes available immediately.  Use
`get_registered_conditions()` or `get_registered_actions()` to list them.

```python
import glfw
from engine import Engine, Scene, GameObject
from engine.logic import EventSystem, Event, KeyPressed, AfterTime, Move

player = GameObject('player.png')
scene = Scene()
scene.add_object(player)

events = EventSystem()
events.add_event(Event([KeyPressed(glfw.KEY_RIGHT)], [Move(player, 5, 0)]))
events.add_event(Event([AfterTime(seconds=5)], [Move(player, -5, 0)]))

Engine(scene=scene, events=events).run()
```

The editor now includes a **Console** dock at the bottom. All output from the
game process and the editor itself appears here so you can easily debug your
scripts. All messages are also written to `logs/editor.log` while the
runtime engine logs to `logs/engine.log` so you can review issues later.
Set the environment variable `SAGE_LOG_LEVEL` to `DEBUG`, `INFO`, `WARNING` or
`ERROR` to control how much detail is recorded. Unknown objects, conditions and
actions generate warnings so issues show up in the log instead of failing silently.
The engine also checks its exported names at import time and warns if a symbol
listed in ``__all__`` does not exist. This helps catch mistakes like missing
imports.
Events can combine many more conditions and actions such as
`KeyReleased`, `MouseButton`, `InputState`, `PlaySound`, `Spawn`,
`ZoomAbove` and `SetZoom`. The *Logic* tab lists
each event with its conditions on the left and actions on the right. The add
event dialog now uses a cleaner grid layout, aligning the condition and action
lists side by side with the Ok/Cancel buttons underneath.

`KeyPressed` and `KeyReleased` also accept a `device` field so the same block
works for keyboard keys or mouse buttons. The improved condition editor lets you
pick the device and specific key from drop‑down lists so you are always in
control of which input triggers the event.

Additional math-friendly blocks make it easy to build counters or timers.
`VariableCompare` tests a variable against a value using operators like `>`,
`<`, or `==`. `ModifyVariable` adjusts a variable with `+`, `-`, `*`, or `/` so
you can implement score systems without custom scripting. Comparisons accept
numeric or boolean variables; strings cannot be compared. Boolean variables
only support `==` and `!=`. When a variable is not numeric the operator box
adapts or disappears with a warning. `PlaySound` now caches each sound after
the first use and reports any errors instead of crashing.

The `Print` action is handy for debugging. The text is formatted with the
current variables so using `Score: {score}` will display the latest value of a
variable named `score` when the action runs.

Numeric fields in conditions and actions may reference engine data at runtime.
Values like ``engine.variable("speed")`` call that method when the event runs so
the latest value is used. Short forms ``$name`` and ``{name}`` are also
recognized for variables. For example ``Move`` can use ``$speed`` as the ``dx``
value to read the current speed variable each frame.

Projects are checked as they load. If an image is missing or a file is
corrupted, the editor reports the problem in the console instead of
terminating.

The logic module registers conditions and actions in dictionaries so new types
can be added without modifying the engine. `register_condition` and
`register_action` accept a small metadata table describing the constructor
arguments. This allows `condition_from_dict` and `action_from_dict` to
instantiate new blocks automatically, making the system easily extensible and
suitable for editor integration. Metadata tuples may also list allowed object
types for parameters that reference a scene object. For example
`('camera', 'object', 'target', ['camera'])` restricts the parameter to Camera
objects while omitting the list accepts any object.

Scene objects use a similar registry. The ``register_object`` decorator
associates each object type with a list of constructor parameters. Functions
``object_from_dict`` and ``object_to_dict`` rely on this metadata so scenes can
load and save new object classes without modifying the loader. All objects,
scenes and projects expose a ``metadata`` dictionary for custom attributes so
tools can store extra information without affecting the runtime.

### SAGE SDK and Plugins

The ``sage_sdk`` package ships with a small plugin loader used by both the
engine and editor.  Directories listed in ``SAGE_PLUGINS`` are scanned for
Python files.  Each module may define ``init_editor(editor)`` and/or
``init_engine(engine)`` functions which are called when the respective
component starts.  Plugins can also be registered programmatically via
``sage_sdk.register_plugin('editor', func)`` or ``'engine'`` for runtime hooks.
This keeps the core lightweight while allowing advanced features to be
packaged separately and installed by simply dropping a file into the plugins
folder.  ``SAGE_ENGINE_PLUGINS`` and ``SAGE_EDITOR_PLUGINS`` provide
component-specific search paths. The editor exposes **Manage Plugins** under the
Settings menu which copies Python files into ``~/.sage_plugins`` and lets you
enable or disable them.

### Versioning

Each project stores the engine version string so upgrades are painless. Older
projects load without issue, and the editor shows the version in its window
title so you always know which release you are using.

### Performance

SAGE Engine aims to run smoothly even on older hardware. Images and sounds
are cached after the first load so repeated objects or effects do not reload
files from disk. The `Engine` class accepts an `fps` argument (default 60) to
control the frame rate using `time.sleep` for consistent timing.
You can clear the image cache with `engine.clear_image_cache()` if memory
becomes tight. These optimizations keep the runtime light without sacrificing
visual quality.

### SAGE API

For small scripts or rapid prototyping the `engine.api` module exposes
helpers to load, save and run projects or scenes with minimal boilerplate:

```python
from engine import (
    load_project, save_project, run_project,
    load_scene, save_scene, run_scene,
)

project = load_project('game.sageproject')
save_project(project, 'copy.sageproject')
run_project('game.sageproject')  # one line to launch
scene = load_scene('level1.json')
save_scene(scene, 'copy.json')
run_scene('level1.json')  # run a single scene file
```

The function `create_engine()` builds an `Engine` from a `Project` while
`load_scene`/`save_scene` and `run_scene` offer the same convenience for raw
scene files.
