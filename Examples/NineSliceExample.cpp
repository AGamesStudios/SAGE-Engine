// ===================================
// 9-Slice System - Пример использования
// ===================================

#include "ECS/ECS.h"
#include "Graphics/Core/Resources/Texture.h"

using namespace SAGE;
using namespace SAGE::ECS;

void Example_CreateUIPanel() {
    Registry registry;
    
    // Загрузить текстуру панели
    auto panelTexture = std::make_shared<Texture>("ui/panel.png");
    
    // === Способ 1: Через компонент напрямую ===
    Entity panel1 = registry.CreateEntity();
    registry.AddComponent(panel1, TransformComponent(100.0f, 100.0f));
    
    // Создать 9-slice компонент (границы 16 пикселей со всех сторон)
    auto& nineSlice = registry.AddComponent<NineSliceComponent>(panel1, panelTexture, 16.0f);
    nineSlice.SetSize(200.0f, 150.0f);  // Размер панели
    nineSlice.SetColor(1.0f, 1.0f, 1.0f, 0.9f);  // Немного прозрачная
    
    // === Способ 2: С разными границами ===
    Entity panel2 = registry.CreateEntity();
    registry.AddComponent(panel2, TransformComponent(350.0f, 100.0f));
    
    // Разные границы (left, right, top, bottom)
    auto& nineSlice2 = registry.AddComponent<NineSliceComponent>(
        panel2, panelTexture, 
        12.0f,  // left
        12.0f,  // right
        20.0f,  // top (заголовок)
        8.0f    // bottom
    );
    nineSlice2.SetSize(300.0f, 200.0f);
    nineSlice2.layer = 1;  // На переднем плане
    
    // === Способ 3: Кнопка ===
    Entity button = registry.CreateEntity();
    registry.AddComponent(button, TransformComponent(100.0f, 300.0f));
    
    auto buttonTexture = std::make_shared<Texture>("ui/button.png");
    auto& buttonSlice = registry.AddComponent<NineSliceComponent>(button, buttonTexture, 8.0f);
    buttonSlice.SetSize(120.0f, 40.0f);
    buttonSlice.sprite.fillCenter = true;  // Заливка центра
    
    // === Добавить систему рендеринга ===
    NineSliceRenderSystem renderSystem;
    // registry.AddSystem(renderSystem);
    
    // === В игровом цикле ===
    // renderSystem.Update(registry, deltaTime);
}

void Example_AnimatedResize() {
    Registry registry;
    
    auto texture = std::make_shared<Texture>("ui/window.png");
    
    Entity window = registry.CreateEntity();
    registry.AddComponent(window, TransformComponent(200.0f, 200.0f));
    auto& nineSlice = registry.AddComponent<NineSliceComponent>(window, texture, 24.0f);
    
    // Изначальный размер
    nineSlice.SetSize(150.0f, 100.0f);
    
    // Анимация изменения размера
    float time = 0.0f;
    float duration = 2.0f;
    
    // В Update():
    // time += deltaTime;
    // float progress = std::min(1.0f, time / duration);
    // float width = 150.0f + progress * 200.0f;  // От 150 до 350
    // float height = 100.0f + progress * 150.0f; // От 100 до 250
    // nineSlice.SetSize(width, height);
}

void Example_DialogueBox() {
    Registry registry;
    
    auto boxTexture = std::make_shared<Texture>("ui/dialogue_box.png");
    
    Entity dialogueBox = registry.CreateEntity();
    registry.AddComponent(dialogueBox, TransformComponent(50.0f, 400.0f));
    
    // Диалоговое окно с толстыми границами
    auto& box = registry.AddComponent<NineSliceComponent>(
        dialogueBox, boxTexture,
        20.0f,  // left (портрет персонажа)
        12.0f,  // right
        16.0f,  // top (заголовок)
        16.0f   // bottom
    );
    
    box.SetSize(700.0f, 150.0f);
    box.sprite.fillCenter = true;
    box.layer = 10;  // Поверх всего
    box.opacity = 0.95f;
}

// ===================================
// Минимальный размер и валидация
// ===================================

void Example_MinimumSize() {
    auto texture = std::make_shared<Texture>("ui/panel.png");
    
    NineSliceSprite sprite(texture, 16.0f, 16.0f, 16.0f, 16.0f);
    
    // Получить минимальный размер
    Vector2 minSize = sprite.GetMinimumSize();
    // minSize.x = 32.0f (16 left + 16 right)
    // minSize.y = 32.0f (16 top + 16 bottom)
    
    // Установить размер меньше минимального - не работает
    sprite.SetSize(20.0f, 20.0f);
    bool valid = sprite.IsValid();  // false
    
    // Установить корректный размер
    sprite.SetSize(100.0f, 80.0f);
    valid = sprite.IsValid();  // true
}

// ===================================
// Использование с UI виджетами
// ===================================

void Example_UIWidget() {
    Registry registry;
    
    auto panelTex = std::make_shared<Texture>("ui/panel.png");
    
    // Создать панель настроек
    Entity settingsPanel = registry.CreateEntity();
    registry.AddComponent(settingsPanel, TransformComponent(300.0f, 150.0f));
    
    auto& panel = registry.AddComponent<NineSliceComponent>(settingsPanel, panelTex, 16.0f);
    panel.SetSize(400.0f, 300.0f);
    panel.layer = 5;
    
    // Можно комбинировать с другими компонентами
    // registry.AddComponent<InteractableComponent>(settingsPanel);
    // registry.AddComponent<DraggableComponent>(settingsPanel);
}
