#include "StageManager.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <utility>

namespace sage2d {

namespace {
    std::string NormalizeKey(const std::string& value) {
        auto trim = [](const std::string& input) {
            std::size_t start = 0;
            std::size_t end = input.size();
            while (start < end && std::isspace(static_cast<unsigned char>(input[start]))) {
                ++start;
            }
            while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
                --end;
            }
            return input.substr(start, end - start);
        };

        auto trimmed = trim(value);
        std::string result;
        result.reserve(trimmed.size());
        for (char ch : trimmed) {
            unsigned char uch = static_cast<unsigned char>(ch);
            if (std::isalnum(uch)) {
                result.push_back(static_cast<char>(std::tolower(uch)));
            }
            else if (ch == '_' || ch == '-' || ch == ' ' || ch == '.') {
                if (!result.empty() && result.back() != '_') {
                    result.push_back('_');
                }
            }
        }
        return result;
    }
}

StageManager::StageManager(Vault& vault)
    : m_Vault(vault) {
}

StageManager::~StageManager() {
    clear();
}

void StageManager::registerStage(const std::string& name, StageFactory factory) {
    auto key = normalizeKey(name);
    m_Factories[key] = std::move(factory);
}

Stage& StageManager::push(const std::string& name) {
    auto key = normalizeKey(name);
    auto it = m_Factories.find(key);
    if (it == m_Factories.end()) {
        throw std::runtime_error("No stage factory registered for " + name);
    }
    auto stage = it->second(m_Vault);
    if (!stage) {
        throw std::runtime_error("Stage factory for " + name + " returned null");
    }
    m_Stack.emplace_back(std::move(stage));
    Stage& ref = *m_Stack.back();
    ref.setStageManager(this);
    applySkinOverride(ref);
    ref.onEnter();
    return ref;
}

Stage& StageManager::push(std::unique_ptr<Stage> stage) {
    if (!stage) {
        throw std::runtime_error("Cannot push null stage");
    }
    m_Stack.emplace_back(std::move(stage));
    Stage& ref = *m_Stack.back();
    ref.setStageManager(this);
    applySkinOverride(ref);
    ref.onEnter();
    return ref;
}

void StageManager::pop() {
    if (m_Stack.empty()) {
        return;
    }
    auto& stage = m_Stack.back();
    stage->onExit();
    stage->setStageManager(nullptr);
    m_Stack.pop_back();
}

Stage& StageManager::replace(const std::string& name) {
    pop();
    return push(name);
}

Stage* StageManager::current() {
    if (m_Stack.empty()) {
        return nullptr;
    }
    return m_Stack.back().get();
}

const Stage* StageManager::current() const {
    if (m_Stack.empty()) {
        return nullptr;
    }
    return m_Stack.back().get();
}

void StageManager::update(float deltaTime) {
    if (Stage* stage = current()) {
        stage->update(deltaTime);
    }
}

void StageManager::clear() {
    while (!m_Stack.empty()) {
        pop();
    }
}

void StageManager::setSkinOverride(const std::string& stageName, ResId skinId) {
    auto key = normalizeKey(stageName);
    if (!IsValid(skinId)) {
        m_SkinOverrides.erase(key);
    }
    else {
        m_SkinOverrides[key] = skinId;
    }

    if (Stage* stage = current()) {
        auto currentKey = normalizeKey(stage->name());
        if (currentKey == key) {
            stage->setDefaultSkin(skinId);
        }
    }
}

std::optional<ResId> StageManager::skinOverride(const std::string& stageName) const {
    auto key = normalizeKey(stageName);
    auto it = m_SkinOverrides.find(key);
    if (it == m_SkinOverrides.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::string StageManager::normalizeKey(const std::string& value) {
    return NormalizeKey(value);
}

void StageManager::applySkinOverride(Stage& stage) {
    auto key = normalizeKey(stage.name());
    auto it = m_SkinOverrides.find(key);
    if (it != m_SkinOverrides.end()) {
        stage.setDefaultSkin(it->second);
    }
}

} // namespace sage2d
