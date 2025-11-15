#include "FunctionRegistry.h"
#include <cmath>
#include <algorithm>
#include <cctype>
#include <random>

namespace SAGE::Scripting::LogCon::Runtime {

// Helper for random number generation
static std::random_device rd;
static std::mt19937 gen(rd());

void RegisterBuiltinFunctions() {
    auto& registry = FunctionRegistry::Get();
    
    // ============================================================================
    // MATH LIBRARY - Математическая библиотека
    // ============================================================================
    FunctionRegistrar("math")
        .Add({"sqrt", "корень", "raiz", "racine", "wurzel", "平方根"}, 
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.empty()) return RuntimeValue(0.0);
                return RuntimeValue(std::sqrt(args[0].AsNumber()));
            })
        .Add({"abs", "модуль", "valor_absoluto", "valeur_absolue", "betrag", "绝对值"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.empty()) return RuntimeValue(0.0);
                return RuntimeValue(std::abs(args[0].AsNumber()));
            })
        .Add({"sin", "синус", "seno", "sinus"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.empty()) return RuntimeValue(0.0);
                return RuntimeValue(std::sin(args[0].AsNumber()));
            })
        .Add({"cos", "косинус", "coseno", "cosinus"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.empty()) return RuntimeValue(0.0);
                return RuntimeValue(std::cos(args[0].AsNumber()));
            })
        .Add({"tan", "тангенс", "tangente", "tangens"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.empty()) return RuntimeValue(0.0);
                return RuntimeValue(std::tan(args[0].AsNumber()));
            })
        .Add({"floor", "пол", "suelo", "sol", "boden", "向下取整"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.empty()) return RuntimeValue(0.0);
                return RuntimeValue(std::floor(args[0].AsNumber()));
            })
        .Add({"ceil", "потолок", "techo", "plafond", "decke", "向上取整"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.empty()) return RuntimeValue(0.0);
                return RuntimeValue(std::ceil(args[0].AsNumber()));
            })
        .Add({"round", "округлить", "redondear", "arrondir", "runden", "四舍五入"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.empty()) return RuntimeValue(0.0);
                return RuntimeValue(std::round(args[0].AsNumber()));
            })
        .Add({"min", "минимум", "minimo", "minimum"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 2) return RuntimeValue(0.0);
                return RuntimeValue(std::min(args[0].AsNumber(), args[1].AsNumber()));
            })
        .Add({"max", "максимум", "maximo", "maximum"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 2) return RuntimeValue(0.0);
                return RuntimeValue(std::max(args[0].AsNumber(), args[1].AsNumber()));
            })
        .Add({"pow", "степень", "potencia", "puissance", "potenz", "幂"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 2) return RuntimeValue(0.0);
                return RuntimeValue(std::pow(args[0].AsNumber(), args[1].AsNumber()));
            });
    
    // ============================================================================
    // STRING LIBRARY - Строковая библиотека
    // ============================================================================
    FunctionRegistrar("string")
        .Add({"length", "длина", "longitud", "longueur", "länge", "长度"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.empty()) return RuntimeValue(0.0);
                return RuntimeValue(static_cast<double>(args[0].AsString().length()));
            })
        .Add({"upper", "заглавные", "mayusculas", "majuscules", "großbuchstaben", "大写"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.empty()) return RuntimeValue("");
                std::string str = args[0].AsString();
                std::transform(str.begin(), str.end(), str.begin(), ::toupper);
                return RuntimeValue(str);
            })
        .Add({"lower", "строчные", "minusculas", "minuscules", "kleinbuchstaben", "小写"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.empty()) return RuntimeValue("");
                std::string str = args[0].AsString();
                std::transform(str.begin(), str.end(), str.begin(), ::tolower);
                return RuntimeValue(str);
            })
        .Add({"contains", "содержит", "contiene", "contient", "enthält", "包含"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 2) return RuntimeValue(false);
                std::string str = args[0].AsString();
                std::string sub = args[1].AsString();
                return RuntimeValue(str.find(sub) != std::string::npos);
            })
        .Add({"substring", "подстрока", "subcadena", "souschaîne", "teilzeichenkette", "子串"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 2) return RuntimeValue("");
                std::string str = args[0].AsString();
                double startDouble = args[1].AsNumber(0.0);
                
                if (startDouble < 0.0) return RuntimeValue("");
                size_t start = static_cast<size_t>(startDouble);
                if (start >= str.length()) return RuntimeValue("");
                
                if (args.size() >= 3) {
                    double lengthDouble = args[2].AsNumber(0.0);
                    if (lengthDouble < 0.0) return RuntimeValue("");
                    size_t length = static_cast<size_t>(lengthDouble);
                    size_t maxLength = str.length() - start;
                    if (length > maxLength) length = maxLength;
                    return RuntimeValue(str.substr(start, length));
                }
                return RuntimeValue(str.substr(start));
            });
    
    // ============================================================================
    // ARRAY LIBRARY - Библиотека массивов
    // ============================================================================
    FunctionRegistrar("array")
        .Add({"size", "размер", "tamaño", "taille", "größe", "大小"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.empty() || !args[0].IsArray()) return RuntimeValue(0.0);
                auto arr = args[0].AsArray();
                return RuntimeValue(arr ? static_cast<double>(arr->size()) : 0.0);
            })
        .Add({"push", "добавить", "agregar", "ajouter", "hinzufügen", "添加"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 2 || !args[0].IsArray()) return RuntimeValue();
                auto arr = args[0].AsArray();
                if (!arr) return RuntimeValue();
                
                constexpr size_t MAX_ARRAY_SIZE = 1000000;
                if (arr->size() >= MAX_ARRAY_SIZE) return RuntimeValue();
                
                arr->push_back(args[1]);
                return RuntimeValue(arr);
            })
        .Add({"pop", "удалить", "eliminar", "supprimer", "entfernen", "删除"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.empty() || !args[0].IsArray()) return RuntimeValue();
                auto arr = args[0].AsArray();
                if (!arr || arr->empty()) return RuntimeValue();
                
                RuntimeValue last = arr->back();
                arr->pop_back();
                return last;
            })
        .Add({"shuffle", "перемешать", "mezclar", "mélanger", "mischen", "洗牌"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.empty() || !args[0].IsArray()) return RuntimeValue();
                auto arr = args[0].AsArray();
                if (!arr) return RuntimeValue();
                
                for (size_t i = arr->size() - 1; i > 0; --i) {
                    std::uniform_int_distribution<size_t> dis(0, i);
                    size_t j = dis(gen);
                    std::swap((*arr)[i], (*arr)[j]);
                }
                return RuntimeValue(arr);
            })
        .Add({"sort", "сортировать", "ordenar", "trier", "sortieren", "排序"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.empty() || !args[0].IsArray()) return RuntimeValue();
                auto arr = args[0].AsArray();
                if (!arr) return RuntimeValue();
                
                std::sort(arr->begin(), arr->end(), 
                    [](const RuntimeValue& a, const RuntimeValue& b) {
                        return a.AsNumber() < b.AsNumber();
                    });
                return RuntimeValue(arr);
            })
        .Add({"find", "найти", "encontrar", "trouver", "finden", "查找"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 2 || !args[0].IsArray()) return RuntimeValue(-1.0);
                auto arr = args[0].AsArray();
                if (!arr) return RuntimeValue(-1.0);
                
                double targetNum = args[1].AsNumber(0.0);
                std::string targetStr = args[1].AsString();
                
                for (size_t i = 0; i < arr->size(); ++i) {
                    if (args[1].IsNumber() && (*arr)[i].AsNumber() == targetNum) {
                        return RuntimeValue(static_cast<double>(i));
                    }
                    if (args[1].IsString() && (*arr)[i].AsString() == targetStr) {
                        return RuntimeValue(static_cast<double>(i));
                    }
                }
                return RuntimeValue(-1.0);
            });
    
    // ============================================================================
    // GAME UTILITIES - Игровые утилиты
    // ============================================================================
    FunctionRegistrar("game")
        .Add({"random", "рандом", "случайное", "aleatorio", "aléatoire", "zufällig", "随机"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                double min = args.size() >= 1 ? args[0].AsNumber(0.0) : 0.0;
                double max = args.size() >= 2 ? args[1].AsNumber(1.0) : 1.0;
                std::uniform_real_distribution<double> dis(min, max);
                return RuntimeValue(dis(gen));
            })
        .Add({"distance", "дистанция", "расстояние", "distancia", "entfernung", "距离"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 4) return RuntimeValue(0.0);
                double x1 = args[0].AsNumber(0.0);
                double y1 = args[1].AsNumber(0.0);
                double x2 = args[2].AsNumber(0.0);
                double y2 = args[3].AsNumber(0.0);
                
                double dx = x2 - x1;
                double dy = y2 - y1;
                return RuntimeValue(std::sqrt(dx * dx + dy * dy));
            })
        .Add({"angle", "угол", "angulo", "winkel", "角度"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 4) return RuntimeValue(0.0);
                double x1 = args[0].AsNumber(0.0);
                double y1 = args[1].AsNumber(0.0);
                double x2 = args[2].AsNumber(0.0);
                double y2 = args[3].AsNumber(0.0);
                
                double dx = x2 - x1;
                double dy = y2 - y1;
                return RuntimeValue(std::atan2(dy, dx) * 180.0 / 3.14159265359);
            })
        .Add({"lerp", "лерп", "интерполяция", "interpolacion", "interpolation"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 3) return RuntimeValue(0.0);
                double a = args[0].AsNumber(0.0);
                double b = args[1].AsNumber(0.0);
                double t = args[2].AsNumber(0.0);
                t = std::clamp(t, 0.0, 1.0);
                return RuntimeValue(a + (b - a) * t);
            })
        .Add({"clamp", "зажать", "ограничить", "limitar", "begrenzen", "限制"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 3) return RuntimeValue(0.0);
                double value = args[0].AsNumber(0.0);
                double minVal = args[1].AsNumber(0.0);
                double maxVal = args[2].AsNumber(1.0);
                return RuntimeValue(std::clamp(value, minVal, maxVal));
            });
    
    // ============================================================================
    // RPG FUNCTIONS - Функции для RPG
    // ============================================================================
    FunctionRegistrar("rpg")
        .Add({"damage", "урон", "daño", "dégât", "schaden", "伤害"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 2) return RuntimeValue(0.0);
                double attack = args[0].AsNumber(0.0);
                double defense = args[1].AsNumber(0.0);
                double damage = attack - defense * 0.5;
                return RuntimeValue(damage > 0.0 ? damage : 0.0);
            })
        .Add({"heal", "лечение", "исцеление", "curar", "guérir", "heilen", "治疗"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 3) return RuntimeValue(0.0);
                double currentHP = args[0].AsNumber(0.0);
                double healAmount = args[1].AsNumber(0.0);
                double maxHP = args[2].AsNumber(100.0);
                double newHP = currentHP + healAmount;
                return RuntimeValue(newHP > maxHP ? maxHP : newHP);
            })
        .Add({"experience", "опыт", "experiencia", "expérience", "erfahrung", "经验"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 2) return RuntimeValue(0.0);
                double level = args[0].AsNumber(1.0);
                double baseXP = args[1].AsNumber(100.0);
                return RuntimeValue(baseXP * std::pow(level, 1.5));
            })
        .Add({"chance", "шанс", "вероятность", "probabilidad", "wahrscheinlichkeit", "概率"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.empty()) return RuntimeValue(false);
                double probability = std::clamp(args[0].AsNumber(0.5), 0.0, 1.0);
                std::uniform_real_distribution<double> dis(0.0, 1.0);
                return RuntimeValue(dis(gen) < probability);
            })
        .Add({"critchance", "крит", "критшанс", "critico", "critique", "kritisch", "暴击"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 2) return RuntimeValue(0.0);
                double baseDamage = args[0].AsNumber(10.0);
                double critRate = std::clamp(args[1].AsNumber(0.1), 0.0, 1.0);
                std::uniform_real_distribution<double> dis(0.0, 1.0);
                return RuntimeValue(dis(gen) < critRate ? baseDamage * 2.0 : baseDamage);
            });
    
    // ============================================================================
    // PLATFORMER FUNCTIONS - Функции для платформеров
    // ============================================================================
    FunctionRegistrar("platformer")
        .Add({"jump", "прыжок", "saltar", "sauter", "springen", "跳跃"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                double jumpPower = args.empty() ? 5.0 : args[0].AsNumber(5.0);
                return RuntimeValue(jumpPower);
            })
        .Add({"gravity", "гравитация", "gravedad", "gravité", "schwerkraft", "重力"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                double gravityStrength = args.empty() ? 0.5 : args[0].AsNumber(0.5);
                return RuntimeValue(gravityStrength);
            })
        .Add({"isgrounded", "наземле", "земля", "ensuelo", "ausol", "amBoden", "在地面"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 2) return RuntimeValue(false);
                double yPos = args[0].AsNumber(0.0);
                double groundLevel = args[1].AsNumber(0.0);
                return RuntimeValue(yPos >= groundLevel);
            });
    
    // ============================================================================
    // SHOOTER FUNCTIONS - Функции для шутеров
    // ============================================================================
    FunctionRegistrar("shooter")
        .Add({"shoot", "выстрел", "disparar", "tirer", "schießen", "射击"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 2) return RuntimeValue(true);
                double ammo = args[0].AsNumber(0.0);
                double fireRate = args[1].AsNumber(1.0);
                return RuntimeValue(ammo > 0.0 && fireRate > 0.0);
            })
        .Add({"reload", "перезарядка", "recargar", "recharger", "nachladen", "重新装填"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                if (args.size() < 2) return RuntimeValue(0.0);
                double maxAmmo = args[1].AsNumber(30.0);
                return RuntimeValue(maxAmmo);
            })
        .Add({"recoil", "отдача", "retroceso", "recul", "rückstoß", "后坐力"},
            [](const std::vector<RuntimeValue>& args, GameObject*) -> RuntimeValue {
                double weaponPower = args.empty() ? 10.0 : args[0].AsNumber(10.0);
                return RuntimeValue(weaponPower * 0.01);
            });
}

} // namespace SAGE::Scripting::LogCon::Runtime
