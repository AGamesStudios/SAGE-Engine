# Sage Command Line

The `sage` command provides basic project utilities. Available subcommands:

* `build`
* `serve`
* `featherize`
* `create <name>` – generate a project from a template

`build`, `serve` and `featherize` remain placeholders. `create` copies the
`minimal_platformer` template when invoked with ``-t platformer`` so you can
start a new game with:

```bash
sage create mygame -t platformer
```

The optional ``--profile`` flag writes a Chrome Trace JSON file capturing the
`Input`, `Patchers`, `Merge` and `Render` phases of a command.
