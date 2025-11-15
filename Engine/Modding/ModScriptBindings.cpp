#include "ModScriptBindings.h"
#include "ModManager.h"
#include <Scripting/LogCon/Runtime/FunctionRegistry.h>

namespace SAGE::Modding {

using namespace SAGE::Scripting::LogCon::Runtime;

void RegisterLogConFunctions() {
    auto& registry = FunctionRegistry::Get();
    
    // Проверка загружен ли мод
    registry.RegisterFunction(
        {"mod_loaded", "мод_загружен", "mod_cargado", "mod_chargé", "mod_geladen", "模组已加载"},
        [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
            if (args.empty()) return RuntimeValue(false);
            std::string modId = args[0].AsString();
            bool loaded = ModManager::Instance().IsModLoaded(modId);
            return RuntimeValue(loaded);
        },
        "modding"
    );
    
    // Получить версию мода
    registry.RegisterFunction(
        {"mod_version", "мод_версия", "mod_version", "version_mod", "mod_version", "模组版本"},
        [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
            if (args.empty()) return RuntimeValue("");
            std::string modId = args[0].AsString();
            const auto* info = ModManager::Instance().GetModInfo(modId);
            if (!info) return RuntimeValue("");
            return RuntimeValue(info->version.ToString());
        },
        "modding"
    );
    
    // Получить имя мода
    registry.RegisterFunction(
        {"mod_name", "мод_имя", "nombre_mod", "nom_mod", "mod_name", "模组名称"},
        [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
            if (args.empty()) return RuntimeValue("");
            std::string modId = args[0].AsString();
            const auto* info = ModManager::Instance().GetModInfo(modId);
            if (!info) return RuntimeValue("");
            return RuntimeValue(info->name);
        },
        "modding"
    );
    
    // Получить автора мода
    registry.RegisterFunction(
        {"mod_author", "мод_автор", "autor_mod", "auteur_mod", "mod_autor", "模组作者"},
        [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
            if (args.empty()) return RuntimeValue("");
            std::string modId = args[0].AsString();
            const auto* info = ModManager::Instance().GetModInfo(modId);
            if (!info) return RuntimeValue("");
            return RuntimeValue(info->author);
        },
        "modding"
    );
    
    // Получить количество загруженных модов
    registry.RegisterFunction(
        {"mods_count", "модов_количество", "cantidad_mods", "nombre_mods", "anzahl_mods", "模组数量"},
        [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
            int count = static_cast<int>(ModManager::Instance().GetLoadedMods().size());
            return RuntimeValue(static_cast<double>(count));
        },
        "modding"
    );
    
    // Разрешить путь к ассету (с учетом mod overrides)
    registry.RegisterFunction(
        {"resolve_asset", "разрешить_ассет", "resolver_activo", "résoudre_actif", "asset_auflösen", "解析资源"},
        [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
            if (args.empty()) return RuntimeValue("");
            std::string assetPath = args[0].AsString();
            std::string resolved = ModManager::Instance().ResolveAssetPath(assetPath);
            return RuntimeValue(resolved);
        },
        "modding"
    );
    
    // Проверить есть ли override для ассета
    registry.RegisterFunction(
        {"has_asset_override", "есть_переопределение", "tiene_override", "a_override", "hat_override", "有资源覆盖"},
        [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
            if (args.empty()) return RuntimeValue(false);
            std::string assetPath = args[0].AsString();
            bool hasOverride = ModManager::Instance().HasAssetOverride(assetPath);
            return RuntimeValue(hasOverride);
        },
        "modding"
    );
}

// Lua bindings будут добавлены позже когда подключим sol2
void RegisterLuaBindings() {
    // TODO: Implement Lua bindings using sol2
    // This will allow Lua scripts to:
    // - mod = Mod.GetInfo("mod_id")
    // - mods = Mod.GetAllLoaded()
    // - path = Mod.ResolveAsset("textures/player.png")
}

} // namespace SAGE::Modding
