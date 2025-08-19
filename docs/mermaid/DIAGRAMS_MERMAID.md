# Mermaid диаграммы

## Обзор
```mermaid
flowchart LR
  core[Core\n(Lifecycle, Window,\nPlugin Loader, Engine Loop)]
  pm[Plugin Manager\nServices / Systems / Commands / Events / Hooks]
  win[Window\n(SDL2 / GLFW / Dummy)]
  plugins[Plugins\n(sage2d, sage3d, sageui,\nsageinput, sagescripts, ...)]

  core -- load --> pm
  pm -- "render(world,camera,window,dt)" --> win
  win -- "events / commands / hooks" --> plugins
```
## Жизненный цикл
```mermaid
flowchart TB
  A[Startup] --> B[Load Plugins] --> C[Build R/W/C]
  C --> D[Main Loop: Systems → Hooks(pre) → Renderer → Hooks(post) → Swap]
  D --> E[Shutdown]
```
## Выбор сервисов
```mermaid
flowchart TB
  A[CLI args\n--profile / --enable / --disable / --render] --> B[Filter by profiles/tags] --> E[Result: World/Camera/Renderer factories]
  PM[Plugin Manager] --> C[Select renderer_factory\n(prefer: sage2d/sage3d)] --> E
```
## Контракты плагина
```mermaid
flowchart TB
  P[Plugin (plugin.py)] --> S1[provide_service\n(renderer_factory / world_factory / ...)]
  P --> S2[register_system\n(name, fn, order)]
  P --> S3[register_command / invoke_command]
  P --> S4[events.on / events.emit]
  P --> S5[register_renderer_hook('pre'|'post')]
```
