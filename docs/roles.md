# 📘 Роли

Роли описывают поведение объектов. Они независимы от сцен и создаются модулем
`objects`, который применяет схемы валидации. Диаграмма связи модулей:

![Scene Object Role](diagrams/scene_object_role.svg)

## 🔹 camera

Простейшая камера.

### 📦 Категории
- **transform**: x, y
- **camera**: zoom

### 🔧 Фазы
update

## 🔹 enemy

### 📦 Категории
- **transform**: x, y, angle
- **sprite**: tex_id, layer, color
- **enemy**: hp, speed

### 🔧 Фазы
update, draw

## 🔹 player

Управляемый игроком персонаж.

### 📦 Категории
- **transform**: x, y, angle
- **sprite**: tex_id, layer, color
- **player**: hp, speed

### 🔧 Фазы
update, draw

## 🔹 sprite

Базовый графический объект с координатами и текстурой.

### 📦 Категории
- **transform**: x, y, angle
- **sprite**: tex_id, layer, color

### 🔧 Фазы
update, draw
