# SAGE FlowScript

FlowScript is a tiny scripting format used for quick object or scene logic.
Each line contains a command followed by arguments. Some keywords also have
Russian synonyms.

Example script:

```
сказать "Привет"
emit hello
```

Use `FlowRunner` to execute scripts or register a task named `flow.run` via the
DAG subsystem.
