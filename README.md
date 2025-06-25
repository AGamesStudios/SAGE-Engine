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
Dock widgets live under ``sage_editor.docks`` while reusable widgets live in
``sage_editor.widgets``. The viewport now uses **QOpenGLWidget** so the
scene you edit is visible inside the editor as well as when running the game.
Embedding the editor in other tools only requires importing these modules.

### Renderer

Rendering now uses a lightweight **OpenGLRenderer**. The same renderer draws the
scene in the editor viewport and when running a project so what you see while
editing matches the game window. A small cross at the scene origin helps
orient objects. When a scene has no objects a small green square is rendered at
the origin so the viewport no longer appears empty. The editor viewport uses its
own camera which can be panned by dragging with the left mouse button. The
cursor is hidden and captured while dragging so the view responds smoothly and
the runtime camera remains unchanged.

### Units and Coordinates

All positions in the engine use **world units** which can be mapped to real
distances. By default one unit equals one pixel, but you can set how many
units represent a meter with ``engine.set_units_per_meter()``.  Helper
functions ``engine.meters()`` and ``engine.kilometers()`` convert distances so
objects can be placed using real-world values. The coordinate system is
**Y-up**, meaning positive ``y`` values move objects upward while negative
values move them down.
When launching the editor a **Project Manager** window appears. It lists your
recent projects with their creation date and full path.  Buttons let you create
a new project, open an existing file or clear the list for a clean start. The
dialog now groups the table under a "Recent Projects" heading with larger
fonts and spacing so paths are easy to read. Right click a project to open it
immediately or remove it from the list. Choosing **Delete** now asks for
confirmation and then removes the entire project folder along with its files.
Once a project is chosen the editor opens maximized in a dark Fusion
theme and provides two
tabs: **Viewport** and **Logic**. The viewport now uses the same
**OpenGLRenderer** as the runtime so what you see while editing
matches the game window. It refreshes roughly thirty times per second so the
editor stays below about 20% CPU usage even on slower machines.
An **Add Object** button beneath the list places a blank object with a default
 name like `New Object`. Every toolbar action and list item loads its icon from
 the `sage_editor/icons` folder, so you can replace these images with your own
 to completely theme the interface. The Import button loads `add.png`, the New
 Folder button uses `folder.png`, the Refresh button uses `refresh.png`, the Run
 action shows `start.png`, the New Project action shows `file.png`, Save Project
 uses `save.png`, the Recent Projects menu displays `recent.png`, the Manage
 Plugins entry uses `plugin.png` and objects show `object.png` or `camera.png`
 depending on their type. Object properties
 can be edited in a dock but there
 is no visual manipulation until rendering support returns.
Projects store a window ``width`` and ``height`` separately from the active
camera size. Scenes can contain multiple cameras. Select a camera in the object
list (or use its context menu) and choose **Set Active Camera** to decide which
one is used. Camera objects now include a **Z** position so they can be layered
with sprites. Projects are saved in a single `.sageproject` file
The object list sits on the right above the Properties panel. Selecting one shows a **Transform**
panel on the right with X, Y, Z, separate Scale X/Y and Rotation fields. A *Link XY* checkbox
lets you keep both scales in sync. When no object is selected the Properties
dock is empty so old values do not linger. The panel scrolls vertically so
every field stays reachable even when the dock is short. Rotation now accounts for non-uniform
scaling so objects spin correctly even when Scale X and Scale Y differ. Scaling
also stays centered on the sprite regardless of rotation. Transform calculations
now use **GLM** so scaling occurs before rotation and objects rotate around
their center without skewing. Each object also defines a **pivot** so the
coordinate system matches the runtime. The transform dock also includes a
**Coordinate Mode** drop-down for switching between *Global* and *Local*
coordinates. Internally every object stores its rotation as a quaternion so
angles remain stable even after many incremental edits. A small **Object**
section above the transform fields lets you rename items and change their type
between *Sprite* and *Camera*. Each object also keeps its own settings
dictionary so properties remain independent across different items. The
project file stores the entire scene
including object positions, events and variables.
The **Resources** dock sits on the left and lists everything under your project's
`resources` folder. Three buttons above the tree (**Import**, **New Folder** and
**Refresh**) sit on the first row while a search field appears below them. The folder button
uses `folder.png` from the `icons` directory. You can drag items
between folders or drop files from outside the application. Dragging files out of
the dock is disabled so resources remain inside the project. Files are sorted alphabetically so projects stay
organized. Any asset chosen from outside the project is copied into this
directory automatically, so scenes never reference files that might disappear.
Imports accept files, folders or even `.zip` archives. A progress dialog shows
the current path and counts bytes so large batches do not freeze the interface.
The window keeps a fixed width so long filenames will not stretch the dialog.
Windows paths are automatically prefixed so deeply nested archives import
without hitting the 260 character limit.
Imported files are loaded immediately into a small cache so previews appear
instantly. Only the most recent images are kept using an LRU scheme so the
cache never grows without bound. The engine loads all assets relative to this
directory, ensuring reorganizing files will not break existing scenes. If PyQt
does not provide ``QFileSystemModel`` the editor falls back to a simpler tree
widget that still lets you create folders and import resources. Right-clicking
any item offers to open it with the default application (icon `open.png`) or
delete it from the project. Hovering an image reveals a floating preview just
offset from the cursor so files are easy to identify. The preview is
confined to the editor window and disappears when it loses focus. Thumbnails are
cached in memory so browsing many files does not lag. Double-clicking a
`.sagescene` file loads it in the editor so you can quickly switch between scenes.
Use **File → New Project** to generate a folder for your game. The dialog asks
for a project name and location. Projects always use the OpenGL renderer so the
editor viewport and runtime look the same.
It then creates the folder with a `.sageproject` file and a `Scenes` subfolder
containing `Scene1.sagescene`. Each new object
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
game start or every frame. Fonts are slightly larger for readability.
Window dimensions can be changed under **Settings → Window Settings**. The
game window title matches the editor, e.g. `SAGE Editor: MyGame - Scene1`.
When you edit the scene the title gains an `(unsaved)` suffix until you save.
Project files record the window `width`, `height` and `title`. When loading a
project the engine creates its window using these values. The active camera
keeps its own resolution. The window can be resized at runtime and the camera
follows the new dimensions while unused space is letterboxed so
the scene maintains its aspect ratio.
When defining variables, boolean values are edited with a convenient check box
instead of typing "true" or "false".
When comparing variables, the name is selected from a drop-down list so typos
are avoided.

Sprites are loaded lazily so the editor does not depend on any particular
window system. The editor also validates image and variable input and
ensures combo boxes always point to valid objects, preventing crashes when
adding multiple sprites, variables, or conditions on older hardware.
Any errors when creating conditions, actions, or variables are caught and
printed to the console instead of closing the editor.
If something goes wrong during gameplay, the console shows the full
Python traceback so you can identify exactly where the problem occurred.
Use the **Clear Log** toolbar button to empty the console at any time.

The editor also provides a **Profiler** dock with graphs for overall CPU,
RAM usage of the editor process, GPU load and the editor process CPU. Frame time in milliseconds is also
tracked so you can see how long each update takes. Sampling only runs while the dock is visible to
keep CPU usage low. The graphs update every second with colored lines so you can monitor performance
without wasting power when the profiler is hidden.

### Localization

The editor supports multiple languages. On startup it selects the language
matching your system locale, but you can change it via the toolbar drop-down.
All strings come from simple dictionaries in `sage_editor/lang.py`, so adding
more languages only requires editing this file. Translations cover file dialogs,
logic windows and event lists so the whole interface is localized. When
choosing assets the editor restricts the file dialog to the project's resources
folder so paths stay valid.
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
or paste the previously copied one. These entries load icons from
`sage_editor/icons` so **Copy**, **Cut**, **Paste** and **Delete** display
`copy.png`, `cut.png`, `paste.png` and `delete.png` respectively. Adding a new
event uses the `add.png` icon for clarity.

The events table now reserves more room for the condition descriptions and
expands rows to fit their text. Right-click the list itself to add a new event
or copy, cut, paste and delete existing ones.

Run a saved project with:

```bash
python -m engine path/to/project.sageproject
```

The editor toolbar provides a **Run** button with the same effect. It saves the
current project and launches it in a separate OpenGL window.


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
the latest value is used. Any engine attribute can be accessed using dotted
paths such as ``engine.camera.zoom`` or ``engine.camera.set_zoom(2)``. Short
forms ``$name`` and ``{name}`` are also recognized for variables. For example
``Move`` can use ``$speed`` as the ``dx`` value to read the current speed
variable each frame.

When editing events, the editor offers auto-completion for any ``engine.``
references. Typing ``engine.`` in a value field shows a list of available engine
methods so you can insert calls without memorizing every name.

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
enable or disable them. Plugin installation is entirely local; the editor does
not download code from the internet.

### Versioning

Each project stores the engine version string so upgrades are painless. Older
projects load without issue, and the editor shows the version in its window
title so you always know which release you are using.

### Performance

SAGE Engine aims to run smoothly even on older hardware. Images and sounds
are cached after the first load and only the most recent 32 images are kept in
memory.  A helper `engine.clear_image_cache()` empties this LRU cache if
memory becomes tight. The `Engine` class accepts an `fps` argument (default 30)
to control the frame rate. `Engine.run()` now launches a Qt **GameWindow** that
updates via a `QTimer` instead of a tight loop, keeping CPU usage low. Object
lists are sorted only when modified and heavy math dependencies were removed.
The editor delays resource searches slightly so typing does not rebuild the tree
on every keystroke. These optimizations keep the runtime light without
sacrificing visual quality.

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
