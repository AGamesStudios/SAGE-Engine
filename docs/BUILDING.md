# Сборка документации

## Установка зависимостей
```bash
pip install -r docs/requirements-docs.txt
```

## Сборка HTML
```bash
# из корня репозитория
sphinx-build -b html docs docs/_build/html
# или
make -C docs html
# Windows
docs\make.bat html
```

Готовые страницы появятся в `docs/_build/html/index.html`.
