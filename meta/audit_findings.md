# Audit Findings

| ID | Subsystem | Severity | Description | Reproduction | Recommendation |
|----|-----------|----------|-------------|--------------|----------------|
| F1 | Window    | S1       | Missing UI backend leads to errors when module listed in engine.json. | Run `core.boot()` with default config. | Remove `ui` from engine.json or implement module. |
| F2 | Audio     | S2       | `Audio.play` is stub, so examples fail to play sound. | Call `audio.play` in example. | Implement basic sound playback or mark as future work. |
| F3 | Shaders   | S2       | `Effect.apply` placeholder, effects pipeline does nothing. | Run effects example. | Provide CPU implementation or disable in docs. |
| F4 | Events    | S3       | `events.boot` and `events.update` are empty leading to queue never flushed automatically. | Emit events then call core.tick. | Implement these hooks or document manual flush. |
| F5 | Visual Tests | S3 | `sage_testing.visual` lacks diff logic so graphical regressions aren't detected. | Run `sage-test`. | Implement screenshot comparison using Pillow. |
| F6 | Core | S1 | Legacy phase system caused startup instability. | Run `python main.py` before cleanup. | Rebuilt minimal core with deterministic phase registry. |

| F7 | Render | S2 | Only software renderer with no display output. | Run `python main.py` and note no window. | Implement real window backends and presentation. |
| F8 | Render | S3 | Software renderer lacked unit tests leading to potential regressions. | N/A | Add unit tests verifying pixel output. |
| F9 | TTY | S2 | No terminal mode for headless environments. | Run engine without window. | Implement TTY subsystem with buffer and input. |
