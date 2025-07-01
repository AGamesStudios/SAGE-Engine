# SAGE Engine

This repository contains **SAGE Engine**, a lightweight core framework for
games.  **SAGE 2D** extends the engine with sprite rendering while **SAGE
Editor** lets you place objects visually and save projects.  The editor remains
small so it runs well even on older computers.  An additional **SAGE Paint**
A lightweight `sage_runtime` package exposes the core engine for standalone games while common utilities live in `sage_sdk`.

tool provides an experimental sprite editor for drawing 2D graphics. The
window shows an "EXPERIMENTAL" banner to highlight this status.  SAGE Paint
supports zooming, right-click panning and an eraser tool so sprites can be
drawn or touched up without leaving the engine.  A live gizmo shows the brush
or eraser size so edits feel precise.  Zoom now follows the cursor position so
the image stays centred on what you are working on.  A vertical toolbar on the
left hosts brush, eraser and fill tools with optional circle or square brush
tips and a smoothing toggle.  Each tool remembers its own width controlled by a
spin box on the main toolbar which also shows the currently selected colour.
Canvas updates are limited to the changed area so even low-end PCs can draw
smoothly without dropping frames.
Undo and redo shortcuts make it easy to revert mistakes.  The window opens
maximised and centres the canvas in its viewport so drawing starts focused on
the artwork.  The canvas sits on a dark gray background with a thin border while
you work.  A new **File** menu can create blank documents, open or save
``.sagepaint`` files and export the current image to PNG alongside the existing
Settings menu for window size.

The current release is **SAGE Engine v0.0.1-alpha**. The engine
stores its version inside every `.sageproject` file so you can safely upgrade
without losing progress.
An **About** entry in the menu shows this version so players know which build
they are running.

## Setup
Install dependencies with:
```bash
pip install -r requirements.txt
```
Or run `scripts/setup.sh` to install them automatically.

## Building an executable
Install PyInstaller and run the build script to create a standalone **SAGE Engine** executable. Place an ``icon.png`` (256×256 is recommended) in ``sage_editor/icons`` to brand the window. If building for Windows also provide ``icon.ico`` converted from that image. The script packages the editor icons so the program has a complete UI when run on another machine. The command below shows the full parameters on one line so it can be copied directly:

```bash
pip install pyinstaller
python -m PyInstaller --name SAGE-Engine --onefile --windowed \
  --icon sage_editor/icons/icon.ico --add-data "sage_editor/icons:sage_editor/icons" main.py
```
Alternatively run `scripts/build_exe.sh` which uses the same command.
The resulting file `dist/SAGE-Engine.exe` can be distributed without needing Python installed.


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
``sage_editor.widgets``. The viewport now uses a ``QOpenGLWidget`` so the
scene you edit is visible inside the editor as well as when running the game.
Embedding the editor in other tools only requires importing these modules.

Additional utilities live under ``engine.tools``.  For example the
``paint`` submodule exposes **SAGE Paint** so you can open it with
``engine.tools.paint.main()`` or import individual widgets like
``engine.tools.paint.Canvas``.

### Renderer

Rendering now uses an **OpenGLRenderer**. Both the editor viewport and the game
window share a Qt ``QOpenGLWidget`` so what you see while editing matches the
running game. A small cross at the origin helps orient
objects and a green square appears when the scene is empty so the viewport never
shows a blank screen. Camera frustums are drawn in yellow and each object gets a
billboard icon pinned to its pivot so items remain visible even without
textures. A white cross follows the mouse so you can see its world position
and stays aligned even when zooming the viewport.
These gizmos are shown only in the viewport; the game window draws
just the scene objects. The viewport has its own camera which can be panned by
dragging with the left mouse button; the cursor is hidden and captured during
the drag so movement feels smooth and the game camera stays untouched. Use the
mouse wheel to zoom the editor camera in or out.
The transform gizmo keeps a constant screen size even on high‑DPI displays and
continues to track the grabbed point while zooming. Its arrows darken when
hovered so you know they can be dragged. A yellow ring allows rotation around
the Z axis and square handles at the ends let you scale objects. A small
toolbar in the viewport corner lets you switch between Move, Rotate and
Scale modes. The game window instead uses
toolbar in the viewport corner lets you switch between Move, Rotate and
Scale modes. The game window instead uses
the Z axis and square handles at the ends let you scale objects. The game window instead uses
the active camera object from the scene so you can inspect levels from any angle
without affecting gameplay.

### Shader Support

Custom effects can use the :class:`~engine.renderers.shader.Shader` helper.
Provide vertex and fragment GLSL source strings or load them from files,
then call :meth:`Shader.compile` to obtain an OpenGL program ID. `GameObject`
instances may reference a shader via ``obj.shader = {'vertex': 'v.glsl',
'fragment': 'f.glsl'}`` and supply a ``shader_uniforms`` dictionary that will be
uploaded whenever the object draws. If no custom shader is set the renderer
falls back to its default textured program.

Camera objects support the same dictionary on ``camera.shader``. When a camera
has a shader assigned, all objects render through that program unless they
specify one of their own. The editor viewport ignores camera shaders so scenes
remain editable.

### Units and Coordinates

All positions in the engine use **world units** which can be mapped to real
distances. By default one unit equals one pixel, but you can set how many
units represent a meter with ``engine.set_units_per_meter()``.  Helper
functions ``engine.meters()`` and ``engine.kilometers()`` convert distances so
objects can be placed using real-world values. The coordinate system is
**Y-up**, meaning positive ``y`` values move objects upward while negative
values move them down.

### Math Utilities

The engine exposes a small ``engine.core.math2d`` module providing
helpers for 2D projects. It supplies quaternion conversions, bounding box
calculations and new functions like ``make_transform`` for building 3×3
matrices, ``transform_point`` to apply them and ``make_ortho`` for
orthographic projection.  ``engine.core.fastmath`` remains as a thin
wrapper for compatibility but now simply re-exports these features.
When launching the editor a **Project Manager** window appears. It lists your
recent projects with their creation date and full path.  Buttons let you create
a new project, open an existing file or clear the list for a clean start. The
dialog now groups the table under a "Recent Projects" heading with larger
fonts and spacing so paths are easy to read. Right click a project to open it
immediately or remove it from the list. Choosing **Delete** now asks for
confirmation and then removes the entire project folder along with its files.
Once a project is chosen the editor opens maximized using a Fusion theme.
You can switch between dark or light mode in the Editor Settings.
The editor provides two
tabs: **Viewport** and **Logic**. The viewport and runtime windows both use an
OpenGL widget so they display the same scene. The viewport refreshes about
sixty times per second and only resizes the renderer when necessary,
keeping CPU usage low even on slower machines.
An **Add Object** button beneath the list places a blank object with a default
 name like `New Object`. Every toolbar action and list item loads its icon from
 the `sage_editor/icons` folder, so you can replace these images with your own
 to completely theme the interface. SAGE Paint shares this same folder for its
 toolbar icons, ensuring all tools draw from one location.
SAGE Paint uses icons named `brush.png`, `eraser.png`, `fill.png`, `undo.png`, `redo.png`, `circle.png`, `square.png`, `colorpicker.png`, `smooth.png`, `zoomin.png` and `zoomout.png` for its toolbar. The Import button loads `add.png`, the New
 Folder button uses `folder.png`, the Refresh button uses `refresh.png`, the Run
 action uses `start.png` and switches to `stop.png` while running. The New Project
 action shows `file.png`, Save Project uses `save.png`, the Tools menu displays
 `tools.png`, View settings uses `settings.png`, the language drop-down shows
 `lang.png`, the Recent Projects menu displays `recent.png`, the Manage Plugins
 entry uses `plugin.png` and objects show `object.png` or `camera.png`
 depending on their type. Object properties
 can be edited in a dock but there
 is no visual manipulation until rendering support returns.
Projects store a window ``width`` and ``height`` separately from the active
camera size. Scenes can contain multiple cameras. Select a camera in the object
list and check **Main Camera** in the properties panel (or use the context menu
action **Set Active Camera**) to decide which one is used. Camera objects now
include a **Z** position so they can be layered with sprites. Projects are saved
in a single `.sageproject` file
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
for a project name and location. Projects use the OpenGL renderer so
the editor viewport and runtime look the same.
It then creates the folder with a `.sageproject` file and a `Scenes` subfolder
containing `Scene1.sagescene`. Each new object
receives a generic name like `New Object (1)` so conditions always target the
correct item. The editor disables sprite, variable and logic actions until a
project is opened, ensuring all changes are saved to a `.sageproject` file.
The **Logic** tab lists object events with
conditions on the left and actions on the right. The event dialog now provides
drop-down lists for keys and shows only parameters relevant to the chosen
action. Variables of type int, float, string and bool can be defined and used
in conditions or actions. Each object stores its own variables so values are not
shared across objects. Public variables appear in the
**Properties** dock where their values can be edited directly. Integers use a spin
box, floats a double spin box, booleans a check box and strings a text field.
Mathematical actions
only apply to int or float variables, ensuring booleans and strings remain
unchanged. When setting a variable in an action the name is
chosen from a drop-down list and booleans use a check box. Events attach to specific objects and can trigger on
game start or every frame. Fonts are slightly larger for readability.
Window dimensions can be changed under **Settings → Project Settings**. The
game window title matches the editor, e.g. `SAGE Editor: MyGame - Scene1`.
When you edit the scene the title gains an `(unsaved)` suffix until you save.
A project's file records the window `width`, `height` and `title`. When loading a
project the engine creates its window using these values. The active camera
keeps its own resolution. The window can be resized at runtime and the camera
follows the new dimensions while unused space is letterboxed so
the scene maintains its aspect ratio. This behavior can be disabled in the
Project Settings if you prefer the view to stretch. Running a project from the
editor uses the same dimensions so what you see matches the final game.
The viewport camera also uses these dimensions so the aspect ratio is identical
while editing.
A **Project Settings** dialog under **Settings** groups options into side tabs.
The dialog uses the Qt *Fusion* style and keeps the tab labels horizontal even
though they sit vertically on the left. The **Info** tab edits the game title,
version (default `0.1.0`) and description while a **Window** tab contains width
and height fields plus the background color. The game title defaults to the name
chosen when creating a project so you only set it once.  The pages scroll inside
a fixed-size window so labels line up neatly.
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
Use the **Clear Log** button in the console to empty it at any time.
Log **Messages**, **Warnings** or **Errors** can be toggled on and off with the
checkboxes provided.
Custom window arrangements can be saved from **Editor → Interface → Save Layout**.
Saved layouts appear in the same menu and selecting one makes it the startup layout. Use **Restore Default** to return to the factory arrangement.
Custom window arrangements can be saved from **Editor → Interface → Save Layout**.
Saved layouts appear in the same menu and selecting one makes it the startup layout. Use **Restore Default** to return to the factory arrangement.

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
current project and opens a new game window using the same OpenGL renderer,
so the scene appears exactly as in the viewport.


Project files store the entire scene so you can share a single file. Use
**File → Save Project** or **File → Save As…** to write your work. The
**Recent Projects** submenu lists the last few files you opened for quick
access. Choose **File → Exit** to close the editor when you are done.
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
you created in the editor. Each scene maintains its own events in addition to
per-object logic and new projects start with a simple "Hello, SAGE!" message
printed to the console when the game launches.

### SAGE Logic Events

The engine includes **SAGE Logic**, a lightweight event system combining
*conditions* and *actions*. Only a few basic blocks are built in such as
`OnStart` and `Print`, leaving most logic to games and plugins which can
register their own using `register_condition` and `register_action`.  Events can be constructed from dictionaries via
`condition_from_dict`, `action_from_dict` and the `event_from_dict` helper
which the editor uses when loading scenes.
`EventSystem.get_event_names()` lists the currently registered events while
methods such as `enable_event()` or `reset_event()` allow scripts to toggle
them at runtime.  Events may belong to *groups* so entire categories can be
enabled, disabled or reset with `enable_group()` and friends.  Each event also
accepts a *priority* number which controls update order; lower numbers run
first so critical logic executes before less important rules.

Event processing only occurs while a :class:`GameWindow` is running so editing a
scene does not trigger logic. The engine updates all events every frame during
gameplay and pauses them automatically when the window closes.

The editor now includes a **Console** dock at the bottom. All output from the
game process and the editor itself appears here so you can easily debug your
scripts. All messages are also written to `logs/editor.log` while the
runtime engine logs to `logs/engine.log` so you can review issues later.
Set the environment variable `SAGE_LOG_LEVEL` to `DEBUG`, `INFO`, `WARNING` or
`ERROR` to control how much detail is recorded. Unknown objects, conditions and
actions generate warnings so issues show up in the log instead of failing silently.
When set to `DEBUG`, additional messages report when scene objects are added or
removed and when events are enabled, disabled or triggered. This output appears
both in the editor's console and the IDE terminal.
The engine also checks its exported names at import time and warns if a symbol
listed in ``__all__`` does not exist. This helps catch mistakes like missing
imports.

Events can be enabled, disabled or reset at runtime using methods on
``EventSystem``. Fields in custom blocks may reference engine data so values
like ``engine.camera.zoom`` are resolved when the event runs.
Conditions and actions can provide a ``reset()`` method which the
engine calls whenever an event is reset to clear any internal state.

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

Objects also use the **SAGE Role** design. Calling ``create_role()`` returns
an object configured for a specific role such as ``empty``, ``sprite`` or
``camera``. Every role shares transform and logic fields while adding its own
options. Sprites include a :class:`Material` describing colour, texture and
simple rendering parameters like ``opacity`` or ``blend`` mode. Camera roles
keep width, height and active status in their metadata.

### SAGE SDK and Plugins

The runtime-only distribution is available through the `sage_runtime` package for games that do not bundle the editor.
It can be launched directly with ``python -m sage_runtime <file>`` or ``python -m engine.runtime <file>`` to run a scene or project without the editor.

The ``sage_sdk`` package ships with a flexible plugin system used by both the
engine and editor.  A :class:`PluginManager` instance handles discovery and
initialization. Directories listed in ``SAGE_PLUGINS`` are scanned for Python
files.  Each module may define ``init_editor(editor)`` and/or
``init_engine(engine)`` functions which are called when the respective
component starts.  Plugins can also be registered programmatically via
``register_plugin`` or by calling ``PluginManager.register()``. This keeps the
core lightweight while allowing advanced features to be packaged separately and
installed by simply dropping a file into the plugins folder.
``SAGE_ENGINE_PLUGINS`` and ``SAGE_EDITOR_PLUGINS`` provide component-specific
search paths. The editor exposes **Manage Plugins** under the Settings menu
which copies Python files into ``~/.sage_plugins`` and lets you enable or
disable them. Plugin installation is entirely local; the editor never downloads
code automatically.

Plugins can also be distributed as standard Python packages. Any entry points
registered under ``sage_engine.plugins`` or ``sage_editor.plugins`` will be
loaded automatically when the engine or editor starts, allowing third-party
packages to extend the engine without touching the plugin folders. Entry points
may return a function, a module providing ``init_engine``/``init_editor`` or a
``PluginBase`` subclass which will be instantiated automatically.

Engine plugins may also define ``register_logic(register_condition, register_action)``
to add custom event blocks. This hook receives the engine's registration
functions so plugins can contribute new conditions and actions without touching
the core logic modules. Any classes registered this way become instantly
available in the editor and ``condition_from_dict`` can instantiate them from
scene files.

### Versioning

Each project records its own **game version** string along with the engine
version used to create it.  Older projects load without issue, and the editor
shows the version in its window title so you always know which release you are
using.

### Performance

SAGE Engine aims to run smoothly even on older hardware. Images and sounds
are cached after the first load and only the most recent 32 images are kept in
memory.  A helper `engine.clear_image_cache()` empties this LRU cache if
memory becomes tight. The `Engine` class accepts an `fps` argument (default 30)
to control the frame rate. `Engine.run()` opens a Qt window and manages
the loop itself, keeping CPU usage low. Object
lists are sorted only when modified and heavy math dependencies were removed.
Runtime errors no longer close the game window automatically; they are logged so
the scene remains visible for inspection.
Expensive transform calculations are accelerated with **Numba** when installed
by decorating them with ``@njit``. The editor delays resource searches slightly
so typing does not rebuild the tree on every keystroke. These optimizations
keep the runtime light without sacrificing visual quality.

Installing `numba` will JIT compile the math helpers and can
significantly speed up large scenes:

```bash
pip install numba
```

### Input Backends

The engine defaults to Qt input handling but also supports PySDL2. Pass
`input_backend='sdl'` to :class:`~engine.core.engine.Engine` to use SDL for
keyboard and mouse events. This allows easy detection of any SDL key and
works without a Qt window. The editor continues to use Qt input.

### Engine Configuration

The :class:`~engine.core.settings.EngineSettings` dataclass groups all
initialization options for the engine. Instead of passing many parameters you
can create a settings object and reuse or modify it:

```python
from engine import Engine, EngineSettings

settings = EngineSettings(width=800, height=600, renderer="opengl")
engine = Engine(settings=settings)
```

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

### Sprite Effects

Objects may include simple effects that react to camera movement.  Effects are
looked up through a registry so new types can be added by calling
``engine.core.register_effect``.  A minimal built-in handler simply offsets a
sprite from its original position:

```python
sprite.effects.append({
    "type": "offset",
    "dx": 10,
    "dy": -5,
})
```
Another built-in `outline` effect draws a border using the given width and color:

```python
sprite.effects.append({
    "type": "outline",
    "width": 4,
    "color": "0,0,0,255",
})
```
Colors may also be provided as hexadecimal strings like ``"#FF8800"`` or
``"#FF8800FF"`` which the renderer automatically converts to RGBA values.
Sprites accept an ``alpha`` attribute from ``0.0`` to ``1.0`` controlling
overall transparency. The value multiplies the alpha channel of the sprite's
``color`` if one is supplied.
Effects are only supported on sprite objects. Retrieve a sprite from a
scene using ``get_object_type`` and append the effect to its ``effects``
list. Cameras ignore such data because they lack an ``effects`` field.
The editor lists active effects in the **Properties** panel so you can edit or
remove them with a single click.

### Post-processing Effects

Cameras can apply post-processing after rendering the scene.  Use
``engine.core.register_post_effect`` to register handlers and list them in
``Camera.post_effects``.  A built-in ``grayscale`` effect converts the final
frame to shades of grey:

```python
camera.post_effects.append({
    "type": "grayscale",
})
```
