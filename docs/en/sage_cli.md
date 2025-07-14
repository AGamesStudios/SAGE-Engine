# Sage Command Line

The `sage` command provides basic project utilities. Available subcommands:

* `build`
* `serve`
* `featherize`
* `create <name>` – generate a project from a template

`build`, `serve` and `featherize` remain placeholders. `create` copies one of
two templates: ``minimal_platformer`` with ``-t platformer`` or
``hello_sprite_py`` for a simple Python script. Start a new project with:

```bash
sage create mygame -t platformer
sage create myhello -t hello_sprite_py
```

The optional ``--profile`` flag writes a Chrome Trace JSON file capturing the
`Input`, `Patchers`, `Merge` and `Render` phases of a command.
