#pragma once

#include "Stage.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace sage2d {

    class StageManager {
    public:
        using StageFactory = std::function<std::unique_ptr<Stage>(Vault&)>;

        explicit StageManager(Vault& vault);
        ~StageManager();

        StageManager(const StageManager&) = delete;
        StageManager& operator=(const StageManager&) = delete;
        StageManager(StageManager&&) = delete;
        StageManager& operator=(StageManager&&) = delete;

        void registerStage(const std::string& name, StageFactory factory);

        Stage& push(const std::string& name);
        Stage& push(std::unique_ptr<Stage> stage);
        void pop();
        Stage& replace(const std::string& name);

        Stage* current();
        const Stage* current() const;

        void update(float deltaTime);
        void clear();

        [[nodiscard]] std::size_t stackSize() const { return m_Stack.size(); }

        void setSkinOverride(const std::string& stageName, ResId skinId);
        [[nodiscard]] std::optional<ResId> skinOverride(const std::string& stageName) const;

    private:
        static std::string normalizeKey(const std::string& value);

        void applySkinOverride(Stage& stage);

        Vault& m_Vault;
        std::vector<std::unique_ptr<Stage>> m_Stack;
        std::unordered_map<std::string, StageFactory> m_Factories;
        std::unordered_map<std::string, ResId> m_SkinOverrides;
    };

} // namespace sage2d
