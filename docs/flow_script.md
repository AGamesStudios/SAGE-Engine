# SAGE FlowScript

FlowScript is a tiny scripting language used for quick object or scene logic.
Each line contains a command followed by arguments. Keywords also have Russian
synonyms so scripts can be written in either language.

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

Use `FlowRunner` to execute scripts or register a task named `flow.run` via the
DAG subsystem.
