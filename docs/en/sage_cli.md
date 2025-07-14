# Sage Command Line

The `sage` command provides basic project utilities. Available subcommands:

* `build` – optional `--bundle <name>` loads configuration from `config/bundles`
* `serve`
* `featherize`
* `create <name>` – generate a project from a template
* `migrate <path>` – update older YAML project files and print a report

`build`, `serve` and `featherize` remain placeholders. `create` copies one of
two templates: ``minimal_platformer`` with ``-t platformer`` or
``hello_sprite_py`` for a simple Python script. Start a new project with:

```bash
sage create mygame -t platformer
sage create myhello -t hello_sprite_py
```

The optional ``--profile`` flag writes a Chrome Trace JSON file capturing the
`Input`, `Patchers`, `Merge` and `Render` phases of a command.
\nUse `sage migrate <project>` to upgrade YAML files from previous versions. JSON input is also supported.
