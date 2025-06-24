# SAGE Engine

This repository contains **SAGE Engine**, a lightweight core framework for
games.  **SAGE 2D** extends the engine with sprite rendering while **SAGE
Editor** lets you place objects visually and save projects.  The editor remains
small so it runs well even on older computers.

## Architecture
The core engine code resides under `sage_engine/core` which defines the
generic `GameObject`, `Scene`, `Engine` and `Project` classes. 2D helpers are
provided in the same package, and the event system lives in
`sage_engine/logic`. `sage_editor` builds on these pieces but remains optional
so games can depend on the engine without pulling in the editor.

### Renderer

Rendering is handled by modules under `sage_engine/renderers`.  The
default `PygameRenderer` draws scenes using pygame, but you can also
use `OpenGLRenderer` for hardware accelerated 2D via PyOpenGL.
Any renderer with the same interface can be plugged in so the core logic
remains independent of the drawing backend.
When launching the editor a **Project Manager** window appears. It lists your
recent projects with their creation date and full path.  Buttons let you create
a new project, open an existing file or clear the list for a clean start.
Right click a project to open it immediately or remove it from the list.
Once a project is chosen the editor opens maximized in a dark Fusion
theme and provides two
tabs: **Viewport** and **Logic**. The viewport uses a virtually infinite
`QGraphicsScene` so you can pan freely. Hold **Ctrl** and scroll to zoom the
view. A configurable grid and axis lines can be toggled from the toolbar. Grid
size and color are adjustable and snapping ensures objects align cleanly when
enabled. A red rectangle shows the canvas (the game window) for reference.
The toolbar also includes an **Add Object** button which places a white square
sprite with a default name like `New Object`. Double-click an object to open a
small editor for changing its image or RGBA color. Clicking a sprite selects it
and the cursor switches between an open
and closed hand while dragging. A yellow gizmo outline appears with a corner
handle for scaling and a small crosshair above for rotation so objects are easy
to manipulate. The gizmo and handles always appear above other sprites so they
remain visible. Projects are saved in a single `.sageproject` file
All objects are listed in a dock on the right. Selecting one shows a **Transform**
panel with X, Y, Z, separate Scale X/Y and Rotation fields. A *Link XY* checkbox
lets you keep both scales in sync. Rotation now accounts for non-uniform
scaling so objects spin correctly even when Scale X and Scale Y differ. The
project file stores the entire scene
including object positions, events and variables.
Use **File → New Project** to generate a folder for your game. The dialog asks
for a project name and a location and then creates the folder with a
`.sageproject` file. Each new object receives a generic name like `New Object (1)` so
conditions always target the correct item. The
editor disables sprite, variable and logic actions until a project is opened,
ensuring all changes are saved to a `.sageproject` file.
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
When defining variables, boolean values are edited with a convenient check box
instead of typing "true" or "false".
When comparing variables, the name is selected from a drop-down list so typos
are avoided.

Sprites are loaded lazily at runtime so the editor no longer relies on
`pygame`'s display module. This prevents crashes when adding images on systems
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

The condition and action lists offer context menus with **Edit**, **Copy**,
**Paste**, and **Delete** options. Right-click an empty area to add a new block
or paste the previously copied one.

Run a saved project with:

```bash
python -m sage_engine path/to/project.sageproject
# pass `--renderer opengl` to test the OpenGL backend
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
```

Sprite positions are stored when you save so the runtime engine can render them
exactly as placed in the editor.
Event definitions are saved as well, so running the scene will include any logic
you created in the editor.

### SAGE Logic Events

The engine ships with **SAGE Logic**, a small condition/action system inspired
by Clickteam. Events consist of *conditions* and *actions*. When all conditions
pass, the actions run. Built-in blocks include `KeyPressed`, `Collision`,
`Timer`, `Move`, `SetPosition`, `Destroy` and `Print`. You can subclass
`Condition` or `Action` to create your own.

```python
import pygame
from sage_engine import Engine, Scene, GameObject
from sage_engine.logic import EventSystem, Event, KeyPressed, Timer, Move

player = GameObject('player.png')
scene = Scene()
scene.add_object(player)

events = EventSystem()
events.add_event(Event([KeyPressed(pygame.K_RIGHT)], [Move(player, 5, 0)]))
events.add_event(Event([Timer(5.0)], [Move(player, -5, 0)]))

Engine(scene=scene, events=events).run()
```

The editor now includes a **Console** dock at the bottom. All output from the
game process and the editor itself appears here so you can easily debug your
scripts. Events can combine many more conditions and actions such as
`KeyReleased`, `MouseButton`, `PlaySound` and `Spawn`. The *Logic* tab lists
each event with its conditions on the left and actions on the right. When adding
an event you choose keys from a drop‑down list and only the relevant parameters
for the selected action are shown so it is quick to create complex behavior.

Additional math-friendly blocks make it easy to build counters or timers.
`VariableCompare` tests a variable against a value using operators like `>`,
`<`, or `==`. `ModifyVariable` adjusts a variable with `+`, `-`, `*`, or `/` so
you can implement score systems without custom scripting. Arithmetic works only
with numeric variables; boolean and string values cannot be modified with math
operations. When such a variable is selected the operator combo box disappears
and a warning icon explains why. `PlaySound` now caches each sound after the
first use and reports any errors instead of crashing.

The `Print` action is handy for debugging. The text is formatted with the
current variables so using `Score: {score}` will display the latest value of a
variable named `score` when the action runs.

Projects are checked as they load. If an image is missing or a file is
corrupted, the editor reports the problem in the console instead of
terminating.

The logic module registers conditions and actions in dictionaries so new types
can be added without modifying the engine. `condition_from_dict` and
`action_from_dict` create objects from saved data, making the system easily
extensible.

### Performance

SAGE Engine aims to run smoothly even on older hardware. Images and sounds
are cached after the first load so repeated objects or effects do not reload
files from disk. The `Engine` class accepts an `fps` argument (default 60) to
control the frame rate and uses `pygame.time.Clock` to limit CPU usage.
You can clear the image cache with `sage_engine.clear_image_cache()` if memory
becomes tight. These optimizations keep the runtime light without sacrificing
visual quality.
