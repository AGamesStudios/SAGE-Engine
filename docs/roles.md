# 📘 Роли

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

## 🔹 rectobject
Простой объект-прямоугольник без текстур.

### 📦 Поля
- x, y, width, height, color

### 🔧 Фазы
draw


## 🔹 sprite

Базовый графический объект с координатами и текстурой.

### 📦 Категории
- **transform**: x, y, angle
- **sprite**: tex_id, layer, color

### 🔧 Фазы
update, draw
