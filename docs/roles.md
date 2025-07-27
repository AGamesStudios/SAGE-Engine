# 📘 Роли

💡 **Роль** определяет поведение и данные объекта. Схема роли описывается через `RoleSchema`, который группирует поля в категории. Категории позволяют логически объединять данные, например `transform` или `sprite`.

### 📦 Пример схемы
```python
RoleSchema(
    name="Enemy",
    categories=[
        Category("transform", [Col("x", "f32", 0.0)]),
        Category("sprite", [Col("texture", "str", "")])
    ],
    vtable=["update", "draw"]
)
```

🔧 Роли регистрируются функцией `register_role(schema)` и могут использоваться в сцене через `SceneEdit.create`.
