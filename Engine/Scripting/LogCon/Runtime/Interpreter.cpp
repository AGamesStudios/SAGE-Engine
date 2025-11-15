#include "Interpreter.h"
#include "FunctionRegistry.h"

#include <Core/GameObject.h>
#include <Core/Logger.h>
#include <Core/Application.h>
#include <Core/Window.h>
#include "../Core/TokenID.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <random>
#include <optional>
#include <unordered_map>

#include <GLFW/glfw3.h>

namespace SAGE::Scripting::LogCon::Runtime {

namespace {

std::string NormalizeIdentifier(const std::string& identifier) {
    std::string result;
    result.reserve(identifier.size());
    for (unsigned char ch : identifier) {
        if (ch >= 'A' && ch <= 'Z') {
            result.push_back(static_cast<char>(ch - 'A' + 'a'));
        } else {
            if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') {
                result.push_back(static_cast<char>(ch));
            }
        }
    }
    return result;
}

int TranslateKeyCode(const std::string& keyName) {
    if (keyName.empty()) {
        return GLFW_KEY_UNKNOWN;
    }

    std::string upper = keyName;
    std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });

    if (upper.size() == 1 && upper[0] >= 'A' && upper[0] <= 'Z') {
        return GLFW_KEY_A + (upper[0] - 'A');
    }

    if (upper == "SPACE") {
        return GLFW_KEY_SPACE;
    }
    if (upper == "ENTER" || upper == "RETURN") {
        return GLFW_KEY_ENTER;
    }
    if (upper == "ESC" || upper == "ESCAPE") {
        return GLFW_KEY_ESCAPE;
    }
    if (upper == "UP" || upper == "ARROWUP") {
        return GLFW_KEY_UP;
    }
    if (upper == "DOWN" || upper == "ARROWDOWN") {
        return GLFW_KEY_DOWN;
    }
    if (upper == "LEFT" || upper == "ARROWLEFT") {
        return GLFW_KEY_LEFT;
    }
    if (upper == "RIGHT" || upper == "ARROWRIGHT") {
        return GLFW_KEY_RIGHT;
    }

    return GLFW_KEY_UNKNOWN;
}

} // namespace

bool Interpreter::Instantiate(std::shared_ptr<const AST::Script> script) {
    Clear();

    if (!script) {
        return false;
    }

    m_Script = std::move(script);
    if (!m_Script) {
        return false;
    }

    for (const auto& entity : m_Script->entities) {
        BuildInstance(entity);
    }

    return !m_Instances.empty();
}

void Interpreter::Clear() {
    for (auto& entry : m_ObjectToInstance) {
        if (auto instance = entry.second.lock()) {
            if (instance->gameObject) {
                instance->gameObject->OnCreate = instance->previousOnCreate;
                instance->gameObject->OnUpdate = instance->previousOnUpdate;
                instance->gameObject->OnDestroy = instance->previousOnDestroy;
            }
            instance->functions.clear();
        }
    }

    m_ObjectToInstance.clear();
    m_Instances.clear();
    m_Script.reset();
}

Interpreter::RuntimeEntityInstance* Interpreter::FindInstance(GameObject* object) {
    if (!object) {
        return nullptr;
    }

    auto it = m_ObjectToInstance.find(object);
    if (it == m_ObjectToInstance.end()) {
        return nullptr;
    }

    return it->second.lock().get();
}

const Interpreter::RuntimeEntityInstance* Interpreter::FindInstance(const GameObject* object) const {
    if (!object) {
        return nullptr;
    }

    auto it = m_ObjectToInstance.find(const_cast<GameObject*>(object));
    if (it == m_ObjectToInstance.end()) {
        return nullptr;
    }

    return it->second.lock().get();
}

std::optional<RuntimeValue> Interpreter::GetProperty(const GameObject* object, const std::string& identifier) const {
    const auto* instance = FindInstance(object);
    if (!instance) {
        return std::nullopt;
    }

    std::string key = NormalizeIdentifier(identifier);
    auto propertyIt = instance->properties.find(key);
    if (propertyIt != instance->properties.end()) {
        return propertyIt->second;
    }

    return ReadGameObjectProperty(*instance, identifier);
}

void Interpreter::BuildInstance(const AST::Entity& entityDefinition) {
    GameObject* object = GameObject::Find(entityDefinition.name);
    if (!object) {
        object = GameObject::Create(entityDefinition.name);
    }

    auto instance = std::make_shared<RuntimeEntityInstance>();
    instance->gameObject = object;
    instance->definition = &entityDefinition;

    for (const auto& event : entityDefinition.events) {
        if (event.type == AST::EventBlock::Type::Custom) {
            std::string key = NormalizeIdentifier(event.eventName);
            instance->customEvents[key] = &event;
        } else {
            instance->events[event.type] = &event;
        }
    }

    RegisterFunctions(*instance);

    for (const auto& statement : entityDefinition.properties) {
        ExecuteStatement(*instance, statement);
    }

    BindEvents(instance);

    m_ObjectToInstance[object] = instance;
    m_Instances.push_back(std::move(instance));
}

void Interpreter::BindEvents(const InstancePtr& instancePtr) {
    if (!instancePtr || !instancePtr->gameObject) {
        return;
    }

    GameObject* object = instancePtr->gameObject;
    std::weak_ptr<RuntimeEntityInstance> weakInstance = instancePtr;

    instancePtr->previousOnCreate = object->OnCreate;
    instancePtr->previousOnUpdate = object->OnUpdate;
    instancePtr->previousOnDestroy = object->OnDestroy;

    // Bind OnCreate
    if (instancePtr->events.find(AST::EventBlock::Type::OnCreate) != instancePtr->events.end()) {
        auto previous = instancePtr->previousOnCreate;
        object->OnCreate = [this, weakInstance, previous]() {
            if (auto instance = weakInstance.lock()) {
                ExecuteEvent(*instance, AST::EventBlock::Type::OnCreate);
            }
            if (previous) {
                previous();
            }
        };
    } else {
        object->OnCreate = instancePtr->previousOnCreate;
    }

    // Bind OnUpdate
    if (instancePtr->events.find(AST::EventBlock::Type::OnUpdate) != instancePtr->events.end()) {
        auto previous = instancePtr->previousOnUpdate;
        object->OnUpdate = [this, weakInstance, previous](float deltaTime) {
            if (auto instance = weakInstance.lock()) {
                ExecuteEvent(*instance, AST::EventBlock::Type::OnUpdate, deltaTime);
            }
            if (previous) {
                previous(deltaTime);
            }
        };
    } else {
        object->OnUpdate = instancePtr->previousOnUpdate;
    }

    // Bind OnDestroy to guarantee cleanup
    bool bindDestroy = instancePtr->events.find(AST::EventBlock::Type::OnDestroy) != instancePtr->events.end();
    auto previous = instancePtr->previousOnDestroy;
    object->OnDestroy = [this, weakInstance, previous, bindDestroy]() {
        if (auto instance = weakInstance.lock()) {
            if (bindDestroy) {
                ExecuteEvent(*instance, AST::EventBlock::Type::OnDestroy);
            }
            UnregisterInstance(instance->gameObject);
        }
        if (previous) {
            previous();
        }
    };
}

void Interpreter::RegisterFunctions(RuntimeEntityInstance& instance) {
    if (!instance.definition) {
        return;
    }

    for (const auto& functionStatement : instance.definition->functions) {
        if (functionStatement.kind != AST::Statement::Kind::FunctionDefinition) {
            continue;
        }

        std::string key = NormalizeIdentifier(functionStatement.functionDefinition.name);
        instance.functions[key] = &functionStatement;
    }
}

void Interpreter::ExecuteBlock(RuntimeEntityInstance& instance, const std::vector<AST::Statement>& statements) {
    for (const auto& statement : statements) {
        ExecuteStatement(instance, statement);
        
        // Stop execution if we hit return/break/continue
        if (instance.execContext.ShouldStop()) {
            break;
        }
    }
}

void Interpreter::ExecuteEvent(RuntimeEntityInstance& instance, AST::EventBlock::Type type) {
    ExecuteEvent(instance, type, 0.0f);
}

void Interpreter::ExecuteEvent(RuntimeEntityInstance& instance, AST::EventBlock::Type type, float deltaTime) {
    auto eventIt = instance.events.find(type);
    if (eventIt == instance.events.end()) {
        return;
    }

    const AST::EventBlock* block = eventIt->second;
    if (!block) {
        return;
    }

    if (!block->parameter.empty()) {
        if (type == AST::EventBlock::Type::OnUpdate) {
            instance.variables[block->parameter] = RuntimeValue(deltaTime);
        } else {
            instance.variables[block->parameter] = RuntimeValue(true);
        }
    }

    if (type == AST::EventBlock::Type::OnUpdate) {
        instance.variables["deltaTime"] = RuntimeValue(deltaTime);
    }

    for (const auto& statement : block->statements) {
        ExecuteStatement(instance, statement);
    }

    if (type == AST::EventBlock::Type::OnUpdate) {
        instance.variables.erase("deltaTime");
    }

    if (!block->parameter.empty()) {
        instance.variables.erase(block->parameter);
    }
}

void Interpreter::ExecuteStatement(RuntimeEntityInstance& instance, const AST::Statement& statement) {
    switch (statement.kind) {
    case AST::Statement::Kind::Assignment:
        ExecuteAssignment(instance, statement.assignment);
        break;
    case AST::Statement::Kind::FunctionCall:
        ExecuteFunctionCall(instance, statement.functionCall);
        break;
    case AST::Statement::Kind::Block:
        ExecuteBlock(instance, statement.block.statements);
        break;
    case AST::Statement::Kind::If: {
        RuntimeValue condition = EvaluateExpression(instance, statement.ifStatement.condition);
        if (condition.AsBool()) {
            ExecuteBlock(instance, statement.ifStatement.thenBranch);
        } else if (!statement.ifStatement.elseBranch.empty()) {
            ExecuteBlock(instance, statement.ifStatement.elseBranch);
        }
        break;
    }
    case AST::Statement::Kind::FunctionDefinition: {
        std::string key = NormalizeIdentifier(statement.functionDefinition.name);
        instance.functions[key] = &statement;
        break;
    }
    case AST::Statement::Kind::VariableDeclaration:
        ExecuteVariableDeclaration(instance, statement.variableDeclaration);
        break;
    case AST::Statement::Kind::TriggerEvent:
        ExecuteTriggerEvent(instance, statement.triggerEvent);
        break;
    case AST::Statement::Kind::Return:
        // Set return value and control flow
        if (statement.returnStatement.value) {
            instance.execContext.returnValue = EvaluateExpression(instance, statement.returnStatement.value);
        }
        instance.execContext.flow = ControlFlow::Return;
        break;
    case AST::Statement::Kind::While: {
        constexpr size_t MAX_ITERATIONS = 1000000; // Защита от бесконечных циклов
        size_t iterationCount = 0;
        
        while (true) {
            // Проверка на превышение лимита итераций
            if (++iterationCount > MAX_ITERATIONS) {
                // Принудительный выход из бесконечного цикла
                break;
            }
            
            RuntimeValue condition = EvaluateExpression(instance, statement.whileLoop.condition);
            if (!condition.AsBool()) {
                break;
            }
            
            ExecuteBlock(instance, statement.whileLoop.body);
            
            // Handle break/continue
            if (instance.execContext.flow == ControlFlow::Break) {
                instance.execContext.Reset();
                break;
            }
            if (instance.execContext.flow == ControlFlow::Continue) {
                instance.execContext.Reset();
                continue;
            }
            if (instance.execContext.flow == ControlFlow::Return) {
                // Return propagates up
                break;
            }
        }
        break;
    }
    case AST::Statement::Kind::For: {
        // для i = 1 до 10 шаг 1 { }
        std::string loopVar = NormalizeIdentifier(statement.forLoop.variable);
        double startVal = EvaluateExpression(instance, statement.forLoop.from).AsNumber();
        double endVal = EvaluateExpression(instance, statement.forLoop.to).AsNumber();
        double stepVal = statement.forLoop.step ? 
            EvaluateExpression(instance, statement.forLoop.step).AsNumber() : 1.0;
        
        // Защита от бесконечных циклов и переполнения
        constexpr size_t MAX_ITERATIONS = 1000000;
        constexpr double MIN_STEP = 0.0000001; // Минимальный шаг
        
        if (std::abs(stepVal) < MIN_STEP) {
            // Шаг слишком мал или равен нулю - бесконечный цикл
            break;
        }
        
        // Оценка количества итераций
        double range = std::abs(endVal - startVal);
        double iterations = range / std::abs(stepVal);
        if (iterations > static_cast<double>(MAX_ITERATIONS)) {
            // Слишком много итераций
            break;
        }
        
        // Save previous value of loop variable
        std::optional<RuntimeValue> previousValue;
        auto it = instance.variables.find(loopVar);
        if (it != instance.variables.end()) {
            previousValue = it->second;
        }
        
        // Execute loop with iteration counter
        size_t iterCount = 0;
        if (stepVal > 0) {
            for (double i = startVal; i <= endVal && iterCount < MAX_ITERATIONS; i += stepVal, ++iterCount) {
                instance.variables[loopVar] = RuntimeValue(i);
                ExecuteBlock(instance, statement.forLoop.body);
                
                if (instance.execContext.flow == ControlFlow::Break) {
                    instance.execContext.Reset();
                    break;
                }
                if (instance.execContext.flow == ControlFlow::Continue) {
                    instance.execContext.Reset();
                    continue;
                }
                if (instance.execContext.flow == ControlFlow::Return) {
                    break;
                }
            }
        } else if (stepVal < 0) {
            for (double i = startVal; i >= endVal && iterCount < MAX_ITERATIONS; i += stepVal, ++iterCount) {
                instance.variables[loopVar] = RuntimeValue(i);
                ExecuteBlock(instance, statement.forLoop.body);
                
                if (instance.execContext.flow == ControlFlow::Break) {
                    instance.execContext.Reset();
                    break;
                }
                if (instance.execContext.flow == ControlFlow::Continue) {
                    instance.execContext.Reset();
                    continue;
                }
                if (instance.execContext.flow == ControlFlow::Return) {
                    break;
                }
            }
        }
        
        // Restore previous value
        if (previousValue.has_value()) {
            instance.variables[loopVar] = previousValue.value();
        } else {
            instance.variables.erase(loopVar);
        }
        break;
    }
    case AST::Statement::Kind::Break:
        instance.execContext.flow = ControlFlow::Break;
        break;
    case AST::Statement::Kind::Continue:
        instance.execContext.flow = ControlFlow::Continue;
        break;
    }
}

void Interpreter::ExecuteAssignment(RuntimeEntityInstance& instance, const AST::Statement::AssignmentData& assignment) {
    RuntimeValue value = EvaluateExpression(instance, assignment.expression);

    // Присваивание элементу массива: arr[index] = value
    if (assignment.isArrayAccess) {
        RuntimeValue target = EvaluateExpression(instance, assignment.targetExpression);
        RuntimeValue indexValue = EvaluateExpression(instance, assignment.indexExpression);
        
        if (!target.IsArray()) {
            // Ошибка: попытка присваивания элементу не-массива
            return;
        }
        
        auto arr = target.AsArray();
        if (!arr) {
            return;
        }
        
        // Безопасная проверка индекса
        double indexDouble = indexValue.AsNumber(0.0);
        if (indexDouble < 0.0 || indexDouble >= static_cast<double>(arr->size())) {
            // Ошибка: индекс вне диапазона или отрицательный
            return;
        }
        
        size_t index = static_cast<size_t>(indexDouble);
        (*arr)[index] = value;
        return;
    }

    std::string key = NormalizeIdentifier(assignment.variable);
    instance.properties[key] = value;

    ApplyPropertyToGameObject(instance, assignment.variable, value);
}

void Interpreter::ExecuteFunctionCall(RuntimeEntityInstance& instance, const AST::Statement::FunctionCallData& call) {
    std::vector<RuntimeValue> arguments;
    arguments.reserve(call.arguments.size());
    for (const auto& argument : call.arguments) {
        arguments.emplace_back(EvaluateExpression(instance, argument));
    }
    CallFunction(instance, call.function, arguments);
}

void Interpreter::ExecuteVariableDeclaration(RuntimeEntityInstance& instance, const AST::Statement::VariableDeclarationData& decl) {
    RuntimeValue value;
    if (decl.initializer) {
        value = EvaluateExpression(instance, decl.initializer);
    }

    std::string key = NormalizeIdentifier(decl.name);

    switch (decl.scope) {
    case AST::Statement::VariableScope::Global:
        m_GlobalVariables[key] = value;
        break;
    case AST::Statement::VariableScope::Constant:
        instance.constants[key] = value;
        break;
    case AST::Statement::VariableScope::Local:
    default:
        instance.variables[key] = value;
        break;
    }
}

void Interpreter::ExecuteTriggerEvent(RuntimeEntityInstance& instance, const AST::Statement::TriggerEventData& trigger) {
    std::string eventName = NormalizeIdentifier(trigger.eventName);

    // Вызов пользовательского события
    auto it = instance.customEvents.find(eventName);
    if (it != instance.customEvents.end() && it->second) {
        const AST::EventBlock* block = it->second;
        
        // Если есть параметр события, сохраняем аргументы
        if (!block->parameter.empty() && !trigger.arguments.empty()) {
            RuntimeValue argValue = EvaluateExpression(instance, trigger.arguments[0]);
            instance.variables[NormalizeIdentifier(block->parameter)] = argValue;
        }

        ExecuteBlock(instance, block->statements);

        // Очистка параметра
        if (!block->parameter.empty()) {
            instance.variables.erase(NormalizeIdentifier(block->parameter));
        }
    } else {
        if (instance.gameObject) {
            SAGE_INFO("LogCon: Entity '{}' triggered custom event '{}'", instance.gameObject->name, trigger.eventName);
        }
    }
}

RuntimeValue Interpreter::CallFunction(RuntimeEntityInstance& instance, const std::string& functionName, const std::vector<RuntimeValue>& arguments) {
    std::string normalizedName = NormalizeIdentifier(functionName);

    if (auto builtin = CallBuiltin(instance, normalizedName, functionName, arguments)) {
        return *builtin;
    }

    auto functionIt = instance.functions.find(normalizedName);
    if (functionIt != instance.functions.end() && functionIt->second) {
        return ExecuteUserFunction(instance, *functionIt->second, arguments);
    }

    if (!functionName.empty()) {
        if (instance.gameObject) {
            SAGE_WARN("LogCon: Unknown function '{}' for entity '{}'", functionName, instance.gameObject->name);
        } else {
            SAGE_WARN("LogCon: Unknown function '{}'", functionName);
        }
    }

    return RuntimeValue();
}

std::optional<RuntimeValue> Interpreter::CallBuiltin(RuntimeEntityInstance& instance, const std::string& normalizedName, const std::string& originalName, const std::vector<RuntimeValue>& arguments) {
    (void)originalName;
    auto* object = instance.gameObject;

    auto logPrefixed = [&](const std::string& message) {
        if (object) {
            SAGE_INFO("[LogCon:{}] {}", object->name, message);
        } else {
            SAGE_INFO("[LogCon] {}", message);
        }
    };

    auto warnUnknownKey = [&](const std::string& keyLabel) {
        if (object) {
            SAGE_WARN("LogCon: Unknown key '{}' for entity '{}'", keyLabel, object->name);
        } else {
            SAGE_WARN("LogCon: Unknown key '{}'", keyLabel);
        }
    };

    // ========================================================================
    // TRY FUNCTION REGISTRY FIRST - Сначала проверяем реестр функций
    // ========================================================================
    auto result = FunctionRegistry::Get().CallFunction(normalizedName, arguments, object);
    if (result.has_value()) {
        return result.value();
    }

    // ========================================================================
    // LEGACY BUILT-IN FUNCTIONS - Старые встроенные функции (GameObject API)
    // ========================================================================
    
    if (normalizedName == "print" || normalizedName == "вывести" || normalizedName == "печать") {
        std::ostringstream stream;
        for (std::size_t i = 0; i < arguments.size(); ++i) {
            if (i > 0) {
                stream << ' ';
            }
            stream << arguments[i].AsString();
        }
        logPrefixed(stream.str());
        return RuntimeValue();
    }

    auto applyMovement = [&](float dx, float dy) {
        if (object) {
            object->MoveBy(dx, dy);
        }
    };

    auto resolveSpeed = [&]() -> float {
        if (arguments.empty()) {
            return 0.0f;
        }
        return static_cast<float>(arguments.back().AsNumber());
    };

    if (normalizedName == "двигатьвверх" || normalizedName == "moveup") {
        float speed = resolveSpeed();
        applyMovement(0.0f, -speed);
        return RuntimeValue();
    }

    if (normalizedName == "двигатьвниз" || normalizedName == "movedown") {
        float speed = resolveSpeed();
        applyMovement(0.0f, speed);
        return RuntimeValue();
    }

    if (normalizedName == "двигатьвлево" || normalizedName == "moveleft") {
        float speed = resolveSpeed();
        applyMovement(-speed, 0.0f);
        return RuntimeValue();
    }

    if (normalizedName == "двигатьвправо" || normalizedName == "moveright") {
        float speed = resolveSpeed();
        applyMovement(speed, 0.0f);
        return RuntimeValue();
    }

    if (normalizedName == "двигать" || normalizedName == "move") {
        if (arguments.size() < 2) {
            return RuntimeValue();
        }
        std::string direction = NormalizeIdentifier(arguments[0].AsString());
        float speed = static_cast<float>(arguments[1].AsNumber());
        if (direction == "вверх" || direction == "up") {
            applyMovement(0.0f, -speed);
        } else if (direction == "вниз" || direction == "down") {
            applyMovement(0.0f, speed);
        } else if (direction == "влево" || direction == "left") {
            applyMovement(-speed, 0.0f);
        } else if (direction == "вправо" || direction == "right") {
            applyMovement(speed, 0.0f);
        }
        return RuntimeValue();
    }

    if (normalizedName == "teleport" || normalizedName == "телепортировать") {
        if (object && arguments.size() >= 2) {
            float x = static_cast<float>(arguments[0].AsNumber(object->x));
            float y = static_cast<float>(arguments[1].AsNumber(object->y));
            object->MoveTo(x, y);
        }
        return RuntimeValue();
    }

    if (normalizedName == "нажатаклавиша" || normalizedName == "keypress" || normalizedName == "keypressed" || normalizedName == "iskeypressed") {
        if (arguments.empty()) {
            return RuntimeValue(false);
        }

        std::string keyName = arguments[0].AsString();
        int keyCode = TranslateKeyCode(keyName);
        
        if (keyCode == GLFW_KEY_UNKNOWN) {
            warnUnknownKey(keyName);
            return RuntimeValue(false);
        }

        // Получаем текущий GLFW контекст (работает и без Application)
        GLFWwindow* window = glfwGetCurrentContext();
        if (!window) {
            // Fallback: пытаемся получить через Application, если он существует
            if (Application::HasInstance()) {
                window = Application::Get().GetWindow().GetNativeWindow();
            }
        }
        
        if (!window) {
            static bool warnedNoWindow = false;
            if (!warnedNoWindow) {
                SAGE_WARN("LogCon: No GLFW window available for key checks");
                warnedNoWindow = true;
            }
            return RuntimeValue(false);
        }

        int state = glfwGetKey(window, keyCode);
        bool isPressed = (state == GLFW_PRESS || state == GLFW_REPEAT);
        
        return RuntimeValue(isPressed);
    }

    if (normalizedName == "вызватьсобытие" || normalizedName == "triggerevent" || normalizedName == "trigger") {
        std::string eventName;
        if (!arguments.empty()) {
            eventName = arguments[0].AsString();
        }
        if (object) {
            SAGE_INFO("LogCon: Entity '{}' triggered scripted event '{}'", object->name, eventName);
        } else {
            SAGE_INFO("LogCon: Triggered scripted event '{}'", eventName);
        }
        return RuntimeValue();
    }

    if (normalizedName == "random" || normalizedName == "случайное") {
        double minValue = 0.0;
        double maxValue = 1.0;
        if (!arguments.empty()) {
            minValue = arguments[0].AsNumber(0.0);
        }
        if (arguments.size() > 1) {
            maxValue = arguments[1].AsNumber(1.0);
        }
        if (minValue > maxValue) {
            std::swap(minValue, maxValue);
        }
        static std::mt19937 rng{std::random_device{}()};
        std::uniform_real_distribution<double> distribution(minValue, maxValue);
        return RuntimeValue(distribution(rng));
    }

    if (normalizedName == "wait" || normalizedName == "ждать") {
        // Coroutine scheduler integration point
        // 
        // Current behavior: No-op (returns immediately)
        // 
        // Planned implementation:
        // 1. Create CoroutineHandle with current execution state
        // 2. Store continuation point (instruction pointer, stack frame)
        // 3. Register with CoroutineScheduler:
        //    - scheduler.Yield(coroutineHandle, waitDuration)
        // 4. Return RuntimeValue::YieldMarker() to signal suspension
        // 5. Resume execution in next frame/after delay via:
        //    - scheduler.ResumeReady() → continues from saved state
        //
        // Example architecture:
        //   class CoroutineScheduler {
        //       void Yield(CoroutineHandle handle, float seconds);
        //       void Update(float deltaTime);  // Resume ready coroutines
        //       bool IsRunning(CoroutineHandle handle);
        //   };
        //
        // For now, wait() is synchronous and completes immediately
        return RuntimeValue();
    }

    // ============================================================================
    // Math Functions
    // ============================================================================
    
    if (normalizedName == "sqrt" || normalizedName == "корень") {
        if (arguments.empty()) return RuntimeValue(0.0);
        return RuntimeValue(std::sqrt(arguments[0].AsNumber()));
    }
    
    if (normalizedName == "abs" || normalizedName == "модуль") {
        if (arguments.empty()) return RuntimeValue(0.0);
        return RuntimeValue(std::abs(arguments[0].AsNumber()));
    }
    
    if (normalizedName == "sin" || normalizedName == "синус") {
        if (arguments.empty()) return RuntimeValue(0.0);
        return RuntimeValue(std::sin(arguments[0].AsNumber()));
    }
    
    if (normalizedName == "cos" || normalizedName == "косинус") {
        if (arguments.empty()) return RuntimeValue(0.0);
        return RuntimeValue(std::cos(arguments[0].AsNumber()));
    }
    
    if (normalizedName == "tan" || normalizedName == "тангенс") {
        if (arguments.empty()) return RuntimeValue(0.0);
        return RuntimeValue(std::tan(arguments[0].AsNumber()));
    }
    
    if (normalizedName == "floor" || normalizedName == "пол") {
        if (arguments.empty()) return RuntimeValue(0.0);
        return RuntimeValue(std::floor(arguments[0].AsNumber()));
    }
    
    if (normalizedName == "ceil" || normalizedName == "потолок") {
        if (arguments.empty()) return RuntimeValue(0.0);
        return RuntimeValue(std::ceil(arguments[0].AsNumber()));
    }
    
    if (normalizedName == "round" || normalizedName == "округлить") {
        if (arguments.empty()) return RuntimeValue(0.0);
        return RuntimeValue(std::round(arguments[0].AsNumber()));
    }
    
    if (normalizedName == "min" || normalizedName == "минимум") {
        if (arguments.empty()) return RuntimeValue(0.0);
        if (arguments.size() == 1) return arguments[0];
        return RuntimeValue(std::min(arguments[0].AsNumber(), arguments[1].AsNumber()));
    }
    
    if (normalizedName == "max" || normalizedName == "максимум") {
        if (arguments.empty()) return RuntimeValue(0.0);
        if (arguments.size() == 1) return arguments[0];
        return RuntimeValue(std::max(arguments[0].AsNumber(), arguments[1].AsNumber()));
    }
    
    if (normalizedName == "pow" || normalizedName == "степень") {
        if (arguments.size() < 2) return RuntimeValue(0.0);
        return RuntimeValue(std::pow(arguments[0].AsNumber(), arguments[1].AsNumber()));
    }

    // ============================================================================
    // String Functions
    // ============================================================================
    
    if (normalizedName == "length" || normalizedName == "длина") {
        if (arguments.empty()) return RuntimeValue(0.0);
        return RuntimeValue(static_cast<double>(static_cast<int>(arguments[0].AsString().length())));
    }
    
    if (normalizedName == "upper" || normalizedName == "заглавные") {
        if (arguments.empty()) return RuntimeValue("");
        std::string str = arguments[0].AsString();
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });
        return RuntimeValue(str);
    }
    
    if (normalizedName == "lower" || normalizedName == "строчные") {
        if (arguments.empty()) return RuntimeValue("");
        std::string str = arguments[0].AsString();
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return RuntimeValue(str);
    }
    
    if (normalizedName == "contains" || normalizedName == "содержит") {
        if (arguments.size() < 2) return RuntimeValue(false);
        std::string haystack = arguments[0].AsString();
        std::string needle = arguments[1].AsString();
        return RuntimeValue(haystack.find(needle) != std::string::npos);
    }
    
    if (normalizedName == "substring" || normalizedName == "подстрока") {
        if (arguments.empty()) return RuntimeValue("");
        std::string str = arguments[0].AsString();
        if (arguments.size() == 1) return RuntimeValue(str);
        
        // Безопасная проверка индекса
        double startDouble = arguments[1].AsNumber(0.0);
        if (startDouble < 0.0 || startDouble >= static_cast<double>(str.length())) {
            return RuntimeValue("");
        }
        
        std::size_t start = static_cast<std::size_t>(startDouble);
        
        if (arguments.size() >= 3) {
            double lengthDouble = arguments[2].AsNumber(static_cast<double>(str.length() - start));
            if (lengthDouble < 0.0) {
                return RuntimeValue("");
            }
            std::size_t length = static_cast<std::size_t>(lengthDouble);
            // Проверка на переполнение
            if (start + length > str.length()) {
                length = str.length() - start;
            }
            return RuntimeValue(str.substr(start, length));
        }
        return RuntimeValue(str.substr(start));
    }
    
    // ============================================================================
    // Array Functions
    // ============================================================================
    
    if (normalizedName == "размер" || normalizedName == "size") {
        if (arguments.empty() || !arguments[0].IsArray()) {
            return RuntimeValue(0.0);
        }
        auto arr = arguments[0].AsArray();
        if (!arr) {
            return RuntimeValue(0.0);
        }
        return RuntimeValue(static_cast<double>(static_cast<int>(arr->size())));
    }
    
    if (normalizedName == "добавить" || normalizedName == "push") {
        if (arguments.size() < 2 || !arguments[0].IsArray()) {
            return RuntimeValue();
        }
        auto arr = arguments[0].AsArray();
        if (!arr) {
            return RuntimeValue();
        }
        // Защита от слишком больших массивов
        constexpr size_t MAX_ARRAY_SIZE = 1000000; // 1 миллион элементов
        if (arr->size() >= MAX_ARRAY_SIZE) {
            return RuntimeValue(); // Отказ от добавления
        }
        arr->push_back(arguments[1]);
        return RuntimeValue(arr);
    }
    
    if (normalizedName == "удалить" || normalizedName == "pop") {
        if (arguments.empty() || !arguments[0].IsArray()) {
            return RuntimeValue();
        }
        auto arr = arguments[0].AsArray();
        if (!arr || arr->empty()) {
            return RuntimeValue();
        }
        RuntimeValue last = arr->back();
        arr->pop_back();
        return last;
    }
    
    // ============================================================================
    // Game Utilities - Общие игровые функции
    // ============================================================================
    
    if (normalizedName == "random" || normalizedName == "рандом" || normalizedName == "случайное") {
        double min = 0.0;
        double max = 1.0;
        if (arguments.size() >= 1) min = arguments[0].AsNumber(0.0);
        if (arguments.size() >= 2) max = arguments[1].AsNumber(1.0);
        
        double range = max - min;
        double random = min + (static_cast<double>(rand()) / RAND_MAX) * range;
        return RuntimeValue(random);
    }
    
    if (normalizedName == "distance" || normalizedName == "дистанция" || normalizedName == "расстояние") {
        if (arguments.size() < 4) return RuntimeValue(0.0);
        double x1 = arguments[0].AsNumber(0.0);
        double y1 = arguments[1].AsNumber(0.0);
        double x2 = arguments[2].AsNumber(0.0);
        double y2 = arguments[3].AsNumber(0.0);
        
        double dx = x2 - x1;
        double dy = y2 - y1;
        return RuntimeValue(std::sqrt(dx * dx + dy * dy));
    }
    
    if (normalizedName == "angle" || normalizedName == "угол") {
        if (arguments.size() < 4) return RuntimeValue(0.0);
        double x1 = arguments[0].AsNumber(0.0);
        double y1 = arguments[1].AsNumber(0.0);
        double x2 = arguments[2].AsNumber(0.0);
        double y2 = arguments[3].AsNumber(0.0);
        
        double dx = x2 - x1;
        double dy = y2 - y1;
        return RuntimeValue(std::atan2(dy, dx) * 180.0 / 3.14159265359);
    }
    
    if (normalizedName == "lerp" || normalizedName == "лерп" || normalizedName == "интерполяция") {
        if (arguments.size() < 3) return RuntimeValue(0.0);
        double a = arguments[0].AsNumber(0.0);
        double b = arguments[1].AsNumber(0.0);
        double t = arguments[2].AsNumber(0.0);
        
        // Clamp t to [0, 1]
        if (t < 0.0) t = 0.0;
        if (t > 1.0) t = 1.0;
        
        return RuntimeValue(a + (b - a) * t);
    }
    
    if (normalizedName == "clamp" || normalizedName == "зажать" || normalizedName == "ограничить") {
        if (arguments.size() < 3) return RuntimeValue(0.0);
        double value = arguments[0].AsNumber(0.0);
        double minVal = arguments[1].AsNumber(0.0);
        double maxVal = arguments[2].AsNumber(1.0);
        
        if (value < minVal) return RuntimeValue(minVal);
        if (value > maxVal) return RuntimeValue(maxVal);
        return RuntimeValue(value);
    }
    
    // ============================================================================
    // RPG Functions - Функции для RPG игр
    // ============================================================================
    
    if (normalizedName == "damage" || normalizedName == "урон") {
        if (arguments.size() < 2) return RuntimeValue(0.0);
        double attack = arguments[0].AsNumber(0.0);
        double defense = arguments[1].AsNumber(0.0);
        
        double damage = attack - defense * 0.5;
        return RuntimeValue(damage > 0.0 ? damage : 0.0);
    }
    
    if (normalizedName == "heal" || normalizedName == "лечение" || normalizedName == "исцеление") {
        if (arguments.size() < 3) return RuntimeValue(0.0);
        double currentHP = arguments[0].AsNumber(0.0);
        double healAmount = arguments[1].AsNumber(0.0);
        double maxHP = arguments[2].AsNumber(100.0);
        
        double newHP = currentHP + healAmount;
        return RuntimeValue(newHP > maxHP ? maxHP : newHP);
    }
    
    if (normalizedName == "experience" || normalizedName == "опыт") {
        if (arguments.size() < 2) return RuntimeValue(0.0);
        double level = arguments[0].AsNumber(1.0);
        double baseXP = arguments[1].AsNumber(100.0);
        
        // XP needed for next level = baseXP * level^1.5
        return RuntimeValue(baseXP * std::pow(level, 1.5));
    }
    
    if (normalizedName == "chance" || normalizedName == "шанс" || normalizedName == "вероятность") {
        if (arguments.empty()) return RuntimeValue(false);
        double probability = arguments[0].AsNumber(0.5);
        
        // Clamp to [0, 1]
        if (probability < 0.0) probability = 0.0;
        if (probability > 1.0) probability = 1.0;
        
        double roll = static_cast<double>(rand()) / RAND_MAX;
        return RuntimeValue(roll < probability);
    }
    
    if (normalizedName == "critchance" || normalizedName == "крит" || normalizedName == "критшанс") {
        if (arguments.size() < 2) return RuntimeValue(false);
        double baseDamage = arguments[0].AsNumber(10.0);
        double critRate = arguments[1].AsNumber(0.1);
        
        double roll = static_cast<double>(rand()) / RAND_MAX;
        if (roll < critRate) {
            return RuntimeValue(baseDamage * 2.0); // Critical hit!
        }
        return RuntimeValue(baseDamage);
    }
    
    // ============================================================================
    // Platformer Functions - Функции для платформеров
    // ============================================================================
    
    if (normalizedName == "jump" || normalizedName == "прыжок") {
        if (arguments.empty()) return RuntimeValue(5.0);
        double jumpPower = arguments[0].AsNumber(5.0);
        return RuntimeValue(jumpPower);
    }
    
    if (normalizedName == "gravity" || normalizedName == "гравитация") {
        if (arguments.empty()) return RuntimeValue(0.5);
        double gravityStrength = arguments[0].AsNumber(0.5);
        return RuntimeValue(gravityStrength);
    }
    
    if (normalizedName == "isgrounded" || normalizedName == "наземле" || normalizedName == "земля") {
        if (arguments.size() < 2) return RuntimeValue(false);
        double yPos = arguments[0].AsNumber(0.0);
        double groundLevel = arguments[1].AsNumber(0.0);
        
        return RuntimeValue(yPos >= groundLevel);
    }
    
    // ============================================================================
    // Shooter Functions - Функции для шутеров
    // ============================================================================
    
    if (normalizedName == "shoot" || normalizedName == "выстрел") {
        if (arguments.size() < 2) return RuntimeValue(true);
        double ammo = arguments[0].AsNumber(0.0);
        double fireRate = arguments[1].AsNumber(1.0);
        
        return RuntimeValue(ammo > 0.0 && fireRate > 0.0);
    }
    
    if (normalizedName == "reload" || normalizedName == "перезарядка") {
        if (arguments.size() < 2) return RuntimeValue(0.0);
        double currentAmmo = arguments[0].AsNumber(0.0);
        double maxAmmo = arguments[1].AsNumber(30.0);
        
        return RuntimeValue(maxAmmo);
    }
    
    if (normalizedName == "recoil" || normalizedName == "отдача") {
        if (arguments.empty()) return RuntimeValue(0.1);
        double weaponPower = arguments[0].AsNumber(10.0);
        
        return RuntimeValue(weaponPower * 0.01); // 1% отдачи
    }
    
    // ============================================================================
    // Puzzle Functions - Функции для головоломок
    // ============================================================================
    
    if (normalizedName == "shuffle" || normalizedName == "перемешать") {
        if (arguments.empty() || !arguments[0].IsArray()) {
            return RuntimeValue();
        }
        auto arr = arguments[0].AsArray();
        if (!arr) return RuntimeValue();
        
        // Fisher-Yates shuffle
        for (size_t i = arr->size() - 1; i > 0; --i) {
            size_t j = rand() % (i + 1);
            std::swap((*arr)[i], (*arr)[j]);
        }
        return RuntimeValue(arr);
    }
    
    if (normalizedName == "sort" || normalizedName == "сортировать") {
        if (arguments.empty() || !arguments[0].IsArray()) {
            return RuntimeValue();
        }
        auto arr = arguments[0].AsArray();
        if (!arr) return RuntimeValue();
        
        // Bubble sort (простой для понимания)
        for (size_t i = 0; i < arr->size(); ++i) {
            for (size_t j = 0; j < arr->size() - 1; ++j) {
                if ((*arr)[j].AsNumber() > (*arr)[j + 1].AsNumber()) {
                    std::swap((*arr)[j], (*arr)[j + 1]);
                }
            }
        }
        return RuntimeValue(arr);
    }
    
    if (normalizedName == "find" || normalizedName == "найти") {
        if (arguments.size() < 2 || !arguments[0].IsArray()) {
            return RuntimeValue(-1.0);
        }
        auto arr = arguments[0].AsArray();
        if (!arr) return RuntimeValue(-1.0);
        
        double targetNum = arguments[1].AsNumber(0.0);
        std::string targetStr = arguments[1].AsString();
        
        for (size_t i = 0; i < arr->size(); ++i) {
            if (arguments[1].IsNumber() && (*arr)[i].AsNumber() == targetNum) {
                return RuntimeValue(static_cast<double>(static_cast<int>(i)));
            }
            if (arguments[1].IsString() && (*arr)[i].AsString() == targetStr) {
                return RuntimeValue(static_cast<double>(static_cast<int>(i)));
            }
        }
        return RuntimeValue(-1.0);
    }

    return std::nullopt;
}


RuntimeValue Interpreter::ExecuteUserFunction(RuntimeEntityInstance& instance, const AST::Statement& functionStatement, const std::vector<RuntimeValue>& arguments) {
    constexpr size_t MAX_RECURSION_DEPTH = 1000; // Защита от переполнения стека
    
    // Проверка глубины рекурсии
    if (instance.recursionDepth >= MAX_RECURSION_DEPTH) {
        // Превышена максимальная глубина рекурсии
        return RuntimeValue();
    }
    
    // Увеличиваем счётчик
    ++instance.recursionDepth;
    
    const auto& data = functionStatement.functionDefinition;

    std::vector<std::string> parameterKeys;
    parameterKeys.reserve(data.parameters.size());

    std::unordered_map<std::string, std::optional<RuntimeValue>> previousValues;
    previousValues.reserve(data.parameters.size());

    for (std::size_t i = 0; i < data.parameters.size(); ++i) {
        const std::string& paramName = data.parameters[i];
        std::string key = NormalizeIdentifier(paramName);
        parameterKeys.push_back(key);

        auto existing = instance.variables.find(key);
        if (existing != instance.variables.end()) {
            previousValues[key] = existing->second;
        } else {
            previousValues[key] = std::nullopt;
        }

        RuntimeValue value;
        if (i < arguments.size()) {
            value = arguments[i];
        }
        instance.variables[key] = std::move(value);
    }

    // Reset control flow before function execution
    instance.execContext.Reset();
    
    ExecuteBlock(instance, data.body);

    // Capture return value before restoring parameters
    RuntimeValue returnValue = instance.execContext.returnValue;
    
    // Clear return control flow (but preserve value)
    instance.execContext.Reset();

    for (const auto& key : parameterKeys) {
        auto previous = previousValues.find(key);
        if (previous != previousValues.end()) {
            if (previous->second.has_value()) {
                instance.variables[key] = previous->second.value();
            } else {
                instance.variables.erase(key);
            }
        } else {
            instance.variables.erase(key);
        }
    }

    // Уменьшаем счётчик рекурсии при выходе из функции
    --instance.recursionDepth;

    return returnValue;
}

RuntimeValue Interpreter::EvaluateExpression(RuntimeEntityInstance& instance, const AST::ExpressionPtr& expression) {
    if (!expression) {
        return RuntimeValue();
    }

    switch (expression->kind) {
    case AST::Expression::Kind::Identifier:
        return ResolveIdentifier(instance, expression->identifier);
    case AST::Expression::Kind::StringLiteral:
        return RuntimeValue(expression->stringValue);
    case AST::Expression::Kind::NumberLiteral:
        return RuntimeValue(expression->numberValue);
    case AST::Expression::Kind::BooleanLiteral:
        return RuntimeValue(expression->boolValue);
    case AST::Expression::Kind::ArrayLiteral: {
        auto arr = std::make_shared<std::vector<RuntimeValue>>();
        arr->reserve(expression->arrayElements.size());
        for (const auto& element : expression->arrayElements) {
            arr->push_back(EvaluateExpression(instance, element));
        }
        return RuntimeValue(arr);
    }
    case AST::Expression::Kind::ArrayAccess: {
        RuntimeValue target = EvaluateExpression(instance, expression->arrayTarget);
        RuntimeValue indexValue = EvaluateExpression(instance, expression->arrayIndex);
        
        if (!target.IsArray()) {
            // Ошибка: попытка индексации не-массива
            return RuntimeValue();
        }
        
        auto arr = target.AsArray();
        if (!arr) {
            return RuntimeValue();
        }
        
        // Безопасная проверка индекса
        double indexDouble = indexValue.AsNumber(0.0);
        if (indexDouble < 0.0 || indexDouble >= static_cast<double>(arr->size())) {
            // Ошибка: индекс вне диапазона или отрицательный
            return RuntimeValue();
        }
        
        size_t index = static_cast<size_t>(indexDouble);
        return (*arr)[index];
    }
    case AST::Expression::Kind::Binary:
        return EvaluateBinaryExpression(instance, expression);
    case AST::Expression::Kind::Unary:
        return EvaluateUnaryExpression(instance, expression);
    case AST::Expression::Kind::Call: {
        std::vector<RuntimeValue> arguments;
        arguments.reserve(expression->callArguments.size());
        for (const auto& argument : expression->callArguments) {
            arguments.push_back(EvaluateExpression(instance, argument));
        }
        return CallFunction(instance, expression->callName, arguments);
    }
    }

    return RuntimeValue();
}

RuntimeValue Interpreter::EvaluateBinaryExpression(RuntimeEntityInstance& instance, const AST::ExpressionPtr& expression) {
    RuntimeValue left = EvaluateExpression(instance, expression->left);
    RuntimeValue right = EvaluateExpression(instance, expression->right);

    switch (expression->binaryOperator) {
    case TokenID::PLUS: {
        if (left.IsString() || right.IsString()) {
            std::ostringstream stream;
            stream << left.AsString() << right.AsString();
            return RuntimeValue(stream.str());
        }
        return RuntimeValue(left.AsNumber() + right.AsNumber());
    }
    case TokenID::MINUS:
        return RuntimeValue(left.AsNumber() - right.AsNumber());
    case TokenID::STAR:
        return RuntimeValue(left.AsNumber() * right.AsNumber());
    case TokenID::SLASH: {
        double divisor = right.AsNumber(1.0);
        if (divisor == 0.0) {
            if (instance.gameObject) {
                SAGE_WARN("LogCon: Division by zero in entity '{}'", instance.gameObject->name);
            } else {
                SAGE_WARN("LogCon: Division by zero");
            }
            return RuntimeValue(0.0);
        }
        return RuntimeValue(left.AsNumber() / divisor);
    }
    case TokenID::MODULO: {
        double divisor = right.AsNumber(1.0);
        if (divisor == 0.0) {
            return RuntimeValue(0.0);
        }
        double value = std::fmod(left.AsNumber(), divisor);
        return RuntimeValue(value);
    }
    case TokenID::EQUAL_EQUAL:
        return RuntimeValue(left.AsString() == right.AsString());
    case TokenID::BANG_EQUAL:
        return RuntimeValue(left.AsString() != right.AsString());
    case TokenID::GREATER:
        return RuntimeValue(left.AsNumber() > right.AsNumber());
    case TokenID::GREATER_EQUAL:
        return RuntimeValue(left.AsNumber() >= right.AsNumber());
    case TokenID::LESS:
        return RuntimeValue(left.AsNumber() < right.AsNumber());
    case TokenID::LESS_EQUAL:
        return RuntimeValue(left.AsNumber() <= right.AsNumber());
    case TokenID::AND:
        return RuntimeValue(left.AsBool() && right.AsBool());
    case TokenID::OR:
        return RuntimeValue(left.AsBool() || right.AsBool());
    default:
        break;
    }

    return RuntimeValue();
}

RuntimeValue Interpreter::EvaluateUnaryExpression(RuntimeEntityInstance& instance, const AST::ExpressionPtr& expression) {
    RuntimeValue operand = EvaluateExpression(instance, expression->operand);

    switch (expression->unaryOperator) {
    case TokenID::MINUS:
        return RuntimeValue(-operand.AsNumber());
    case TokenID::NOT:
        return RuntimeValue(!operand.AsBool());
    default:
        break;
    }

    return operand;
}

RuntimeValue Interpreter::ResolveIdentifier(RuntimeEntityInstance& instance, const std::string& identifier) {
    std::string key = NormalizeIdentifier(identifier);

    // 1. Проверяем локальные переменные
    auto variableIt = instance.variables.find(key);
    if (variableIt != instance.variables.end()) {
        return variableIt->second;
    }

    // 2. Проверяем константы
    auto constantIt = instance.constants.find(key);
    if (constantIt != instance.constants.end()) {
        return constantIt->second;
    }

    // 3. Проверяем глобальные переменные
    auto globalIt = m_GlobalVariables.find(key);
    if (globalIt != m_GlobalVariables.end()) {
        return globalIt->second;
    }

    // 4. Проверяем свойства сущности
    auto propertyIt = instance.properties.find(key);
    if (propertyIt != instance.properties.end()) {
        return propertyIt->second;
    }

    // 5. Проверяем свойства GameObject
    if (auto objectValue = ReadGameObjectProperty(instance, identifier)) {
        return *objectValue;
    }

    // 6. Неизвестный идентификатор - возвращаем как символьную константу
    return RuntimeValue(identifier);
}

void Interpreter::ApplyPropertyToGameObject(RuntimeEntityInstance& instance, const std::string& identifier, const RuntimeValue& value) {
    if (!instance.gameObject) {
        return;
    }

    GameObject* object = instance.gameObject;
    std::string normalized = NormalizeIdentifier(identifier);

    auto applyFloat = [&](float& target) {
        target = static_cast<float>(value.AsNumber(target));
    };

    auto applyBool = [&](bool& target) {
        target = value.AsBool();
    };

    if (normalized == "x") {
        applyFloat(object->x);
        return;
    }
    if (normalized == "y") {
        applyFloat(object->y);
        return;
    }
    if (normalized == "width" || identifier == "ширина") {
        applyFloat(object->width);
        return;
    }
    if (normalized == "height" || identifier == "высота") {
        applyFloat(object->height);
        return;
    }
    if (normalized == "angle") {
        applyFloat(object->angle);
        return;
    }
    if (normalized == "layer" || identifier == "слой") {
        object->layer = static_cast<int>(value.AsNumber(object->layer));
        return;
    }
    if (normalized == "visible" || identifier == "видимый") {
        applyBool(object->visible);
        return;
    }
    if (normalized == "alpha") {
        applyFloat(object->alpha);
        return;
    }
    if (normalized == "physics" || identifier == "физика") {
        applyBool(object->physics);
        return;
    }
    if (normalized == "speedx") {
        applyFloat(object->speedX);
        return;
    }
    if (normalized == "speedy") {
        applyFloat(object->speedY);
        return;
    }
    // Убрали маппинг "скорость" -> speedX/speedY, чтобы это было просто свойство скрипта
    // if (normalized == "скорость" || normalized == "speed") {
    //     float baseSpeed = static_cast<float>(value.AsNumber());
    //     object->speedX = baseSpeed;
    //     object->speedY = baseSpeed;
    //     return;
    // }
    if (normalized == "gravity" || identifier == "гравитация") {
        applyFloat(object->gravity);
        return;
    }
    if (normalized == "friction" || identifier == "трение") {
        applyFloat(object->friction);
        return;
    }
}

std::optional<RuntimeValue> Interpreter::ReadGameObjectProperty(const RuntimeEntityInstance& instance, const std::string& identifier) const {
    if (!instance.gameObject) {
        return std::nullopt;
    }

    const GameObject* object = instance.gameObject;
    std::string normalized = NormalizeIdentifier(identifier);

    if (normalized == "x") {
        return RuntimeValue(object->x);
    }
    if (normalized == "y") {
        return RuntimeValue(object->y);
    }
    if (normalized == "width" || identifier == "ширина") {
        return RuntimeValue(object->width);
    }
    if (normalized == "height" || identifier == "высота") {
        return RuntimeValue(object->height);
    }
    if (normalized == "angle") {
        return RuntimeValue(object->angle);
    }
    if (normalized == "layer" || identifier == "слой") {
        return RuntimeValue(static_cast<double>(object->layer));
    }
    if (normalized == "visible" || identifier == "видимый") {
        return RuntimeValue(object->visible);
    }
    if (normalized == "alpha") {
        return RuntimeValue(object->alpha);
    }
    if (normalized == "physics" || identifier == "физика") {
        return RuntimeValue(object->physics);
    }
    if (normalized == "speedx") {
        return RuntimeValue(object->speedX);
    }
    if (normalized == "speedy") {
        return RuntimeValue(object->speedY);
    }
    // Убрали чтение "скорость" из GameObject - это теперь просто переменная скрипта
    // if (normalized == "скорость" || normalized == "speed") {
    //     float magnitude = std::sqrt(object->speedX * object->speedX + object->speedY * object->speedY);
    //     return RuntimeValue(magnitude);
    // }
    if (normalized == "gravity" || identifier == "гравитация") {
        return RuntimeValue(object->gravity);
    }
    if (normalized == "friction" || identifier == "трение") {
        return RuntimeValue(object->friction);
    }

    return std::nullopt;
}

void Interpreter::UnregisterInstance(GameObject* object) {
    if (!object) {
        return;
    }

    auto mapIt = m_ObjectToInstance.find(object);
    if (mapIt != m_ObjectToInstance.end()) {
        if (auto instance = mapIt->second.lock()) {
            m_Instances.erase(std::remove_if(m_Instances.begin(), m_Instances.end(), [&](const InstancePtr& ptr) {
                return ptr.get() == instance.get();
            }), m_Instances.end());
        }
        m_ObjectToInstance.erase(mapIt);
    }
}

} // namespace SAGE::Scripting::LogCon::Runtime
