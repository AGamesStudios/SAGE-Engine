# Demo Game

This example demonstrates a minimal game using FlowScript for input-driven logic.
Window parameters and the script path come from `game.sagecfg`.
Run it with:

```bash
python examples/demo_game/main.py
```

The game window shows a cyan rectangle that can be moved left and right with the arrow keys. Logic is written entirely in `logic.flow`.

Example configuration `game.sagecfg`:

```cfg
название = "FlowScript Demo"
ширина = 640
высота = 360
скрипт = "logic.flow"
```

Excerpt from `logic.flow`:

```flow
при обновление сделай
    если нажата_клавиша "влево" тогда
        изменить позицию -5 по X
конец
```
