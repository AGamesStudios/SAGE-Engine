# Feather-FX Shader System

`Feather-FX` loads `.sage_fx` files describing simple shader passes. Each file
starts with a `PASS` section followed by operations. Comments begin with `#`.

Example file:

```
PASS
blit
blend_add factor:0.5
outline color:#FF0000 radius:2
```

Use `load_fx(path)` to parse and cache the effect and `apply_fx(surface, fx)` to
run it. The runtime automatically picks the backend using the
`FEATHER_FX_BACKEND` environment variable. Only a stub CPU implementation is
currently available.

A minimal GPU backend stub generates GLSL 1.20 fragments. Set
`FEATHER_FX_BACKEND=gpu` to see the generated shader. The CPU interpreter is
not implemented yet (see `# FIXME retro_backend`).

Run the FX Lab example:

```bash
python examples/fx_lab/main.py
```
