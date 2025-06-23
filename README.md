# SAGE 2D Engine

This repository provides a small 2D game engine with a scene editor. It uses
`pygame` for rendering and PyQt6 for the editor interface. Scenes consist of
movable sprite objects that you can place visually using the **SAGE Editor**.
Extra care is taken to keep the editor lightweight so it runs well even on
older computers.

## Architecture
The engine core lives in the `sage_engine` package. It provides `GameObject`, `Scene`, and `Engine` classes while the separate `sage_logic` module supplies conditions and actions. The `sage_editor` GUI builds on these pieces but remains optional so games can use the engine without the editor. This separation keeps the code easy to extend and reuse.
The editor launches maximized in a dark Fusion
theme and provides two
tabs: **Viewport** and **Logic**. The viewport uses a virtually infinite
`QGraphicsScene` so you can pan freely. A red rectangle shows the canvas (the
game window) for reference. You can add sprites, drag them around and
save/load scenes as JSON files. Each sprite receives a unique name like
`enemy (1)` if necessary so conditions always target the correct object.
The **Logic** tab lists object events with
conditions on the left and actions on the right. The event dialog now provides
drop-down lists for keys and shows only parameters relevant to the chosen
action. Variables of type int, float, string and bool can be defined and used
in conditions or actions. Mathematical actions only apply to int or float
variables, ensuring booleans and strings remain unchanged. When setting a variable in an action the name is
chosen from a drop-down list and booleans use a check box. Events attach to specific objects and can trigger on
game start or every frame. Fonts are slightly larger for readability. Use the
*Run* button in the toolbar to launch the current scene directly from the
editor.
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

### Localization

The editor supports multiple languages. A toolbar at the top lets you choose
between **English** and **Русский**. All strings come from simple dictionaries
in `sage_editor/lang.py`, making it easy to add more languages by updating the
file. Translations now cover file dialogs and event windows so the interface is fully localized.

The condition and action lists offer context menus with **Edit**, **Copy**,
**Paste**, and **Delete** options. Right-click an empty area to add a new block
or paste the previously copied one.

Run a saved scene with:

```bash
python -m sage_engine path/to/scene.json
```

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
from sage_logic import EventSystem, Event, KeyPressed, Timer, Move

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

The logic module registers conditions and actions in dictionaries so new types
can be added without modifying the engine. `condition_from_dict` and
`action_from_dict` create objects from saved data, making the system easily
extensible.
