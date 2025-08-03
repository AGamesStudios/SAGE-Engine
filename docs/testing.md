# 📘 SAGE Testing

Модуль `sage_engine.testing` объединяет инструменты для юнит-, интеграционных
и визуальных тестов. Запуск производится через `sage-test`.

Основные возможности:
- `assert_.deep_structure(obj)`
- `assert_.role_attached(obj, "Role")`
- `visual.diff("sprite.png", "sprite.png")`
- `flowtest.assert_path_passed(flow, ["a", "b"])`

`visual.diff` может сравнивать скриншоты с учётом режима сглаживания,
что удобно для тестов `graphic`.

`performance.assert_fps` логирует кадры в reports/ и может быть использован
для анализа дрифта при работе `FrameSync`.
