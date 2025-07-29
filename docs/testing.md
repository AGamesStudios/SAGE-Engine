# 📘 SAGE Testing

Модуль `sage_engine.testing` объединяет инструменты для юнит-, интеграционных
и визуальных тестов. Запуск производится через `sage-test`.

Основные возможности:
- `assert_.deep_structure(obj)`
- `assert_.role_attached(obj, "Role")`
- `visual.diff(exp.png, act.png)`
- `flowtest.assert_path_passed(flow, ["a", "b"])`

`visual.diff` может сравнивать скриншоты с учётом режима сглаживания,
что удобно для тестов `graphic`.
