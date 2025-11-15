#pragma once

#include "RuntimeValue.h"
#include "../Core/AST.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace SAGE {
class GameObject;
}

namespace SAGE::Scripting::LogCon::Runtime {

// Control flow state for return/break/continue
enum class ControlFlow {
    None,       // Normal execution
    Return,     // Function returned
    Break,      // Loop break
    Continue    // Loop continue
};

struct ExecutionContext {
    ControlFlow flow = ControlFlow::None;
    RuntimeValue returnValue;  // Value from return statement
    size_t iterationCount = 0; // Защита от бесконечных циклов
    
    void Reset() {
        flow = ControlFlow::None;
        returnValue = RuntimeValue();
    }
    
    bool ShouldStop() const {
        return flow != ControlFlow::None;
    }
};

class Interpreter {
public:
    struct RuntimeEntityInstance {
        GameObject* gameObject = nullptr;
        const AST::Entity* definition = nullptr;
        std::unordered_map<std::string, RuntimeValue> properties;
        std::unordered_map<std::string, RuntimeValue> variables;      // локальные переменные
        std::unordered_map<std::string, RuntimeValue> constants;      // константы
        std::unordered_map<AST::EventBlock::Type, const AST::EventBlock*> events;
        std::unordered_map<std::string, const AST::EventBlock*> customEvents;  // пользовательские события
        std::function<void()> previousOnCreate;
        std::function<void(float)> previousOnUpdate;
        std::function<void()> previousOnDestroy;
        std::unordered_map<std::string, const AST::Statement*> functions;
        
        ExecutionContext execContext;  // Control flow state
        size_t recursionDepth = 0;     // Защита от переполнения стека
    };

    Interpreter() = default;

    bool Instantiate(std::shared_ptr<const AST::Script> script);
    void Clear();

    RuntimeEntityInstance* FindInstance(GameObject* object);
    [[nodiscard]] const RuntimeEntityInstance* FindInstance(const GameObject* object) const;
    [[nodiscard]] std::optional<RuntimeValue> GetProperty(const GameObject* object, const std::string& identifier) const;

    // Глобальные переменные (доступны всем сущностям)
    std::unordered_map<std::string, RuntimeValue>& GetGlobalVariables() { return m_GlobalVariables; }

private:
    using InstancePtr = std::shared_ptr<RuntimeEntityInstance>;

    void BuildInstance(const AST::Entity& entityDefinition);
    void BindEvents(const InstancePtr& instance);
    void ExecuteEvent(RuntimeEntityInstance& instance, AST::EventBlock::Type type);
    void ExecuteEvent(RuntimeEntityInstance& instance, AST::EventBlock::Type type, float deltaTime);
    void RegisterFunctions(RuntimeEntityInstance& instance);
    void ExecuteBlock(RuntimeEntityInstance& instance, const std::vector<AST::Statement>& statements);

    void ExecuteStatement(RuntimeEntityInstance& instance, const AST::Statement& statement);
    void ExecuteAssignment(RuntimeEntityInstance& instance, const AST::Statement::AssignmentData& assignment);
    void ExecuteFunctionCall(RuntimeEntityInstance& instance, const AST::Statement::FunctionCallData& call);
    void ExecuteVariableDeclaration(RuntimeEntityInstance& instance, const AST::Statement::VariableDeclarationData& decl);
    void ExecuteTriggerEvent(RuntimeEntityInstance& instance, const AST::Statement::TriggerEventData& trigger);
    RuntimeValue CallFunction(RuntimeEntityInstance& instance, const std::string& functionName, const std::vector<RuntimeValue>& arguments);
    std::optional<RuntimeValue> CallBuiltin(RuntimeEntityInstance& instance, const std::string& normalizedName, const std::string& originalName, const std::vector<RuntimeValue>& arguments);
    RuntimeValue ExecuteUserFunction(RuntimeEntityInstance& instance, const AST::Statement& functionStatement, const std::vector<RuntimeValue>& arguments);

    [[nodiscard]] RuntimeValue EvaluateExpression(RuntimeEntityInstance& instance, const AST::ExpressionPtr& expression);
    [[nodiscard]] RuntimeValue EvaluateBinaryExpression(RuntimeEntityInstance& instance, const AST::ExpressionPtr& expression);
    [[nodiscard]] RuntimeValue EvaluateUnaryExpression(RuntimeEntityInstance& instance, const AST::ExpressionPtr& expression);
    [[nodiscard]] RuntimeValue ResolveIdentifier(RuntimeEntityInstance& instance, const std::string& identifier);
    void ApplyPropertyToGameObject(RuntimeEntityInstance& instance, const std::string& identifier, const RuntimeValue& value);
    [[nodiscard]] std::optional<RuntimeValue> ReadGameObjectProperty(const RuntimeEntityInstance& instance, const std::string& identifier) const;

    void UnregisterInstance(GameObject* object);

    std::shared_ptr<const AST::Script> m_Script;
    std::vector<InstancePtr> m_Instances;
    std::unordered_map<GameObject*, std::weak_ptr<RuntimeEntityInstance>> m_ObjectToInstance;
    std::unordered_map<std::string, RuntimeValue> m_GlobalVariables;  // глобальные переменные
};

} // namespace SAGE::Scripting::LogCon::Runtime
