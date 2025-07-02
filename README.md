# SAGE Engine

This repository contains **SAGE Engine**, a lightweight core framework for
games.  **SAGE 2D** extends the engine with sprite rendering while **SAGE
Editor** lets you place objects visually and save projects.  The editor remains
small so it runs well even on older computers. An additional **SAGE Paint** tool
provides an experimental sprite editor for drawing 2D graphics. A lightweight
`sage_runtime` package exposes the core engine for standalone games while common
utilities live in `sage_sdk`. See the files under `docs/` for detailed
instructions and tutorials.

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
Use ``engine.require_version()`` to ensure your project runs on a compatible
engine release.

## Setup
Install dependencies with:
```bash
pip install -r requirements.txt
```
Or run `scripts/setup.sh` to install them automatically.

### Running tests
Execute `PYTHONPATH=. pytest` to run the automated test suite which
checks the engine for common errors and regressions. You can also place
the following snippet in `pytest.ini` so the path is set automatically:

```ini
[pytest]
pythonpath = .
```

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
`sage_editor` to keep them modular.  The engine never imports the editor so
projects can depend on the runtime alone while the editor builds entirely on the
public API.  The ``sage_sdk`` package provides shared
utilities like the plugin loader used by both components.
The editor is fully modular. ``sage_editor.gui`` provides the main window
and viewport widgets, plus a simple console showing log output beneath the view.
Startup logic lives in ``sage_editor.app`` which configures a Qt ``Fusion``
style for a modern look. Plugins registered via ``sage_editor.plugins`` can
extend the interface with custom actions without changing the core code.
Embedding the editor in other tools only requires importing these modules.

The runtime is organised in a simple hierarchy:

```
SAGE Engine -> Project -> Scenes -> Objects -> properties
```
Projects load resources through `engine.core.resources.ResourceManager` which
uses a global **SAGE Cache** so data is reused between scenes. Debug messages
are written to `logs/engine.log` using the helpers in `engine.utils.log`.
Call `engine.utils.log.init_logger()` once at startup to create this file.
You can change the verbosity at runtime with
`engine.utils.log.set_level('DEBUG')`.
When importing zip archives the resource manager skips entries that would
extract outside the resources folder so malicious archives cannot overwrite
arbitrary files.

Additional utilities live under ``engine.tools``.  For example the
``paint`` submodule exposes **SAGE Paint** so you can open it with
``engine.tools.paint.main()`` or import individual widgets like
``engine.tools.paint.Canvas``.
Each tool package also reports its own ``__version__`` so scripts can
check compatibility.

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
dragging with the left mouse button. Click without moving to select the object
under the cursor. Use the mouse wheel to zoom the editor camera in or out.
The transform gizmo keeps a constant screen size even on high‑DPI displays and
continues to track the grabbed point while zooming. Its arrows darken when
hovered so you know they can be dragged. A yellow ring allows rotation around
the Z axis and square handles at the ends let you scale objects. A small
toolbar in the viewport corner lets you switch between Move, Rotate and
Scale modes. The game window instead uses the active camera object from
the scene so you can inspect levels from any angle without affecting
gameplay.

``OpenGLRenderer`` accepts ``samples`` and ``vsync`` parameters so games can
choose an MSAA level and control vertical sync. Call
``renderer.grab_image()`` or ``renderer.save_screenshot()`` to capture the
current frame.

For systems without graphics acceleration the engine includes a lightweight
``NullRenderer``. It performs no drawing so games can run headless or on
very old PCs. Pass ``renderer="null"`` to :class:`engine.core.engine.Engine` or
use ``engine.renderers.NullRenderer`` directly when constructing the engine.

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
The editor opens directly to a single viewport using the same OpenGL renderer as the runtime so even low-end machines maintain consistent performance.
Sprites are loaded lazily so the editor does not depend on any particular
window system. The editor also validates image and variable input and
ensures combo boxes always point to valid objects, preventing crashes when
adding multiple sprites, variables, or conditions on older hardware.
Any errors when creating conditions, actions, or variables are caught and
Python traceback so you can identify exactly where the problem occurred.
Log **Messages**, **Warnings** or **Errors** can be toggled on and off with the
checkboxes provided.

RAM usage of the editor process, GPU load and the editor process CPU. Frame time in milliseconds is also
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
python main.py path/to/project.sageproject
# or
python -m sage_editor path/to/scene.json
# or from Python
python -c "import sage_editor as ed; ed.main(['game.sageproject'])"
```
Supply a project or scene path to open it directly; otherwise a blank
scene is created with a single object at the origin.

Sprite positions are stored when you save so the runtime engine can render them
exactly as placed in the editor.
Event definitions are saved as well, so running the scene will include any logic
you created in the editor. Each scene maintains its own events in addition to
per-object logic and new projects start with a simple "Hello, SAGE!" message

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

Object roles can be expanded by plugins. Any package exposing an entry point
named ``sage_engine.objects`` that returns a class will be registered
automatically, allowing scenes to use custom objects without modifying the core
engine.

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
code automatically. The plugin loader also checks that every ``.py`` file
resides within the configured directory, ignoring symlinks that point outside
to avoid executing unexpected code.

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

Logic plugins distributed as standard packages may also expose an entry point
called ``sage_engine.logic``. Any modules listed under this group are imported
when :mod:`engine.logic` is first imported so their conditions and actions are
registered automatically.

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
``input_backend='sdl'`` to :class:`~engine.core.engine.Engine` to use SDL for
keyboard and mouse events. This allows easy detection of any SDL key and
works without a Qt window. The editor continues to use Qt input.

### SAGE Input

``engine.inputs`` provides a flexible ``InputManager`` that maps actions to
keys or mouse buttons and dispatches press/release callbacks. Axes can be
defined with positive and negative keys for analog-like control. A lightweight
``NullInput`` backend is bundled for testing or headless systems.

### Engine Configuration

The :class:`~engine.core.settings.EngineSettings` dataclass groups all
initialization options for the engine. Instead of passing many parameters you
can create a settings object and reuse or modify it:

```python
from engine import Engine, EngineSettings

settings = EngineSettings(width=800, height=600, renderer="opengl")
engine = Engine(settings=settings)
```

`Engine` instances expose a ``delta_time`` attribute which stores the time in
seconds since the last frame. The main loop clamps this value to
``max_delta`` (``0.1`` by default) so logic remains stable even when a slow
frame occurs on older hardware.

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

## License

This project is released under the [MIT License](LICENSE).

