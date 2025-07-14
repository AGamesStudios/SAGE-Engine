# Patcher Scripts

Patchers update the runtime state every frame. Scripts can be written in
either **Python** or **Lua**. Use `engine.load_patcher()` to load a script file
that defines an `apply(tree)` function. Lua patchers require the optional
`lupa` package and will be skipped if LuaJIT is unavailable.
