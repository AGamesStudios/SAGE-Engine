#include "Localization.h"

namespace SAGE {
namespace Editor {

Localization::Localization() {
    using enum TextID;

    auto setEntry = [this](TextID id, const char* english, const char* russian) {
        const auto index = static_cast<std::size_t>(id);
        m_Entries[index].english = english;
        m_Entries[index].russian = russian;
    };

    setEntry(Menu_File, "File", "Файл");
    setEntry(Menu_NewScene, "New Scene", "Новая сцена");
    setEntry(Menu_OpenScene, "Open Scene...", "Открыть сцену...");
    setEntry(Menu_SaveScene, "Save Scene", "Сохранить сцену");
    setEntry(Menu_SaveSceneAs, "Save Scene As...", "Сохранить сцену как...");
    setEntry(Menu_Exit, "Exit", "Выход");
    setEntry(Menu_View, "View", "Вид");
    setEntry(Menu_Viewport, "Viewport", "Вьюпорт");
    setEntry(Menu_Hierarchy, "Hierarchy", "Иерархия");
    setEntry(Menu_Inspector, "Inspector", "Инспектор");
    setEntry(Menu_Help, "Help", "Помощь");
    setEntry(Menu_About, "About", "О программе");
    setEntry(Menu_RecentProjects, "Recent Projects", "Последние проекты");
    setEntry(Menu_ClearRecent, "Clear List", "Очистить список");
    setEntry(Menu_HelpShortcuts, "Keyboard Shortcuts", "Горячие клавиши");
    setEntry(Menu_HelpDocs, "Documentation", "Документация");
    setEntry(Menu_Language, "Language", "Язык");
    setEntry(Language_English, "English", "Английский");
    setEntry(Language_Russian, "Russian", "Русский");
    setEntry(Viewport_WindowTitle, "Viewport", "Вьюпорт");

    setEntry(SceneStatus_NewSceneCreated, "New scene created", "Создана новая сцена");
    setEntry(SceneStatus_SpecifyScenePath, "Specify a scene file path", "Укажите путь к файлу сцены");
    setEntry(SceneStatus_LoadFailed, "Failed to load scene", "Не удалось загрузить сцену");
    setEntry(SceneStatus_Loaded, "Scene loaded: {}", "Сцена загружена: {}");
    setEntry(SceneStatus_NoActiveScene, "No active scene", "Нет активной сцены");
    setEntry(SceneStatus_SpecifySavePath, "Specify a path to save", "Укажите путь для сохранения");
    setEntry(SceneStatus_SaveFailed, "Failed to save scene", "Не удалось сохранить сцену");
    setEntry(SceneStatus_Saved, "Scene saved: {}", "Сцена сохранена: {}");

    setEntry(SceneLabel_Format, "Scene: {}", "Сцена: {}");
    setEntry(SceneLabel_NewPlaceholder, "<new>", "<новая>");

    setEntry(Dialog_OpenScene_Title, "Open Scene", "Открыть сцену");
    setEntry(Dialog_SaveScene_Title, "Save Scene", "Сохранить сцену");
    setEntry(Dialog_OpenScene_Prompt, "Enter path to a scene JSON file", "Введите путь к JSON-файлу сцены");
    setEntry(Dialog_SaveScene_Prompt, "Enter path to save the scene", "Введите путь для сохранения сцены");
    setEntry(Dialog_Open_Button, "Open", "Открыть");
    setEntry(Dialog_Save_Button, "Save", "Сохранить");
    setEntry(Dialog_Cancel_Button, "Cancel", "Отмена");
    setEntry(Dialog_PathEmptyError, "Path must not be empty", "Путь не должен быть пустым");

    setEntry(Inspector_WindowTitle, "Inspector", "Инспектор");
    setEntry(Inspector_NoEntitySelected, "No entity selected", "Сущность не выбрана");
    setEntry(Inspector_SelectedEntityMissing, "Selected entity not found", "Выбранная сущность не найдена");
    setEntry(Inspector_EntityLabel, "Entity", "Сущность");
    setEntry(Inspector_IDLabel, "ID", "ID");
    setEntry(Inspector_TransformHeader, "Transform", "Трансформ");
    setEntry(Inspector_SpriteHeader, "Sprite", "Спрайт");
    setEntry(Inspector_Position, "Position", "Позиция");
    setEntry(Inspector_Rotation, "Rotation", "Поворот");
    setEntry(Inspector_Scale, "Scale", "Масштаб");
    setEntry(Inspector_NoTransform, "No Transform component", "Отсутствует компонент Transform");
    setEntry(Inspector_AddTransform, "Add Transform", "Добавить Transform");
    setEntry(Inspector_Visible, "Visible", "Видимый");
    setEntry(Inspector_FlipX, "Flip X", "Отразить по X");
    setEntry(Inspector_FlipY, "Flip Y", "Отразить по Y");
    setEntry(Inspector_Size, "Size", "Размер");
    setEntry(Inspector_Tint, "Tint", "Цвет");
    setEntry(Inspector_TextureLabel, "Texture: {}", "Текстура: {}");
    setEntry(Inspector_TextureNone, "<none>", "<нет>");
    setEntry(Inspector_LoadTexture, "Load Texture...", "Загрузить текстуру...");
    setEntry(Inspector_ClearTexture, "Clear Texture", "Очистить текстуру");
    setEntry(Inspector_NoSprite, "No Sprite component", "Отсутствует компонент Sprite");
    setEntry(Inspector_AddSprite, "Add Sprite", "Добавить Sprite");
    setEntry(Inspector_AddComponent, "Add Component", "Добавить компонент");
    setEntry(Inspector_TextureDialog_Title, "Texture Loading", "Загрузка текстуры");
    setEntry(Inspector_TextureDialog_Prompt, "Enter texture path (UTF-8)", "Введите путь к текстуре (UTF-8)");
    setEntry(Inspector_TextureDialog_Submit, "Load", "Загрузить");
    setEntry(Inspector_TextureDialog_Cancel, "Cancel", "Отмена");
    setEntry(Inspector_TextureDialog_LoadFailed, "Failed to load texture", "Не удалось загрузить текстуру");

    setEntry(Hierarchy_WindowTitle, "Hierarchy", "Иерархия");
    setEntry(Hierarchy_NoScene, "No scene loaded", "Сцена не загружена");
    setEntry(Hierarchy_CreateEntity, "Create Entity", "Создать сущность");
    setEntry(Hierarchy_DefaultEntityName, "Entity", "Сущность");
    setEntry(Hierarchy_DefaultSpriteName, "Sprite", "Спрайт");
    setEntry(Hierarchy_NoEntities, "No entities", "Нет сущностей");
    setEntry(Hierarchy_ContextRename, "Rename", "Переименовать");
    setEntry(Hierarchy_ContextDelete, "Delete", "Удалить");
}

std::string_view Localization::GetLanguageCode(Language language) {
    switch (language) {
    case Language::Russian:
        return "ru";
    case Language::English:
    default:
        return "en";
    }
}

Language Localization::FromLanguageCode(std::string_view code) {
    if (code == "ru" || code == "ru-RU") {
        return Language::Russian;
    }
    return Language::English;
}

} // namespace Editor
} // namespace SAGE
