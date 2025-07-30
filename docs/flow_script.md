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

## Bytecode

`compile_source()` converts FlowScript to Python source which is compiled to a code object. `compile_to_bytes()` serializes that code object so it can be stored in a `.sageflow` file. `FlowRuntime` can execute either plain source or the serialized bytecode using `run_bytecode()`.

*** End Patch
