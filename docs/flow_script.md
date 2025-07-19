# SAGE FlowScript

FlowScript is a tiny scripting language used for quick object or scene logic.
Commands are defined in ``grammar.yaml`` and can be aliased in multiple
languages. Additional command sets can be loaded from ``flow_modules``.

Example script:

```
сказать "Привет"
emit hello
```

Variables are declared with `var` and can be modified using `add`, `sub`,
`mul` and `div`. Use `@global` to store them between script runs.

Control flow statements (`if`, `else`, `while`, `loop`) rely on indentation just
like Python:

```
@global
var health = 100
if health < 50:
    log "Low HP"
    set color "player" 1 0 0
```

Scripts can manipulate scene objects using commands such as `create Sprite id`
and `set pos "id" x y`.

These commands map to the [logic API](docs/logic_api.md) used by Lua scripts.

Use `FlowRunner` to execute scripts or register a task named `flow.run` via the
DAG subsystem.

Run ``from sage_fs import load_grammar`` to inspect the available commands.
