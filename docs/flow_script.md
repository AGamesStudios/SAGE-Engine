# 📘 FlowScript

FlowScript is a tiny scripting language for SAGE Engine. It is designed to look like natural text and compiles to compact bytecode. The same constructs are available in Russian and English.

## Keywords

| Русский                 | English            | Purpose                     |
|-------------------------|--------------------|-----------------------------|
| `переменная`, `пусть`   | `variable`, `let`  | Variable declaration        |
| `функция`               | `function`         | Function definition         |
| `конец`                 | `end`              | Block terminator            |
| `если`                  | `if`               | Conditional start           |
| `иначе`                 | `else`             | Conditional alternative     |
| `повторить ... раз`     | `repeat ... times` | Simple loop                 |
| `при обновление сделай` | `on update do`     | Update event                |
| `при событие`           | `on event`         | Custom event handler        |
| `вызвать`               | `call`             | Call a function             |
| `прибавить`             | `add`              | Addition                    |
| `уменьшить`             | `subtract`         | Subtraction                 |
| `умножить`              | `multiply`         | Multiplication              |
| `разделить`             | `divide`           | Division                    |
| `нажата`, `клавиша_нажата`, `нажата_клавиша` | `pressed`, `key_pressed`, `pressed_key` | Key pressed (is_down) |
| `зажата` | `held` | Key held (is_pressed) |
| `отпущена` | `released` | Key released (is_up) |

## Grammar

FlowScript is translated line by line into simplified Python. Every command is written on a separate line with indentation preserved. `конец`/`end` simply closes the previous block by returning to a lower indentation level.

### Example

```flow
пусть score = 0
функция add_score()
    прибавить score на 1
конец

при обновление сделай
    если score < 10 тогда
        вызвать add_score()
    иначе
        завершить игру()
конец
```

The same in English:

```flow
let score = 0
function add_score()
    add score by 1
end

on update do
    if score < 10 then
        call add_score()
    else
        end_game()
end
```

### Checking keyboard input

```flow
при обновление сделай
    если зажата "LEFT" тогда
        move(player, -5, 0)
конец
```

If a wrong key name is used, an error is logged:

```
[ERROR] [flow] Неизвестная клавиша или действие: 'LFT'
[HINT] [flow] Возможно вы имели в виду 'LEFT'
```

## \U0001F465 Group commands

FlowScript can manipulate object groups using commands like:

```flow
группа.создать("враги")
группа.добавить(объект)
группа.отключить_логику("враги")
```

These map to :mod:`objects.groups` API and allow batch logic operations.

## Bytecode

`compile_source()` converts FlowScript to Python source which is compiled to a code object. `compile_to_bytes()` serializes that code object so it can be stored in a `.sageflow` file. `FlowRuntime` can execute either plain source or the serialized bytecode using `run_bytecode()`.

*** End Patch
