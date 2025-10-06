#include "Stage.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <utility>
#include <vector>

namespace sage2d {

[[nodiscard]] bool EventTargetsObject(ObjectId objectId, const Event& event) {
    const EventPayload& payload = event.payload;
    if (payload.target != kInvalidObjectId && payload.target != objectId &&
        payload.sender != objectId && payload.other != objectId) {
        return false;
    }
    return true;
}

namespace {
    constexpr int kMaxPhysicsStepsPerFrame = 8;

    std::string NormalizeKey(std::string value) {
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

Stage::Stage(std::string name, Vault& vault)
    : m_Name(std::move(name))
    , m_Vault(vault) {
}

Stage::~Stage() {
    clear();
    m_EventBus.clear();
    m_Contacts.clear();
    m_Timers.clear();
    if (IsValid(m_DefaultSkin)) {
        m_Vault.release(m_DefaultSkin);
        m_DefaultSkin = kInvalidResId;
    }
}

void Stage::setStageManager(StageManager* manager) {
    m_Manager = manager;
}

void Stage::setOnEnter(std::function<void(Stage&)> callback) {
    m_OnEnter = std::move(callback);
}

void Stage::setOnExit(std::function<void(Stage&)> callback) {
    m_OnExit = std::move(callback);
}

void Stage::onEnter() {
    if (m_OnEnter) {
        m_OnEnter(*this);
    }
}

void Stage::onExit() {
    if (m_OnExit) {
        m_OnExit(*this);
    }
}

void Stage::setFixedDelta(float stepSeconds) {
    if (stepSeconds > 0.0f && std::isfinite(stepSeconds)) {
        m_FixedDelta = stepSeconds;
    }
}

void Stage::setGravity(Vec2 gravity) {
    m_Gravity = gravity;
}

void Stage::setDefaultSkin(ResId skinId) {
    if (m_DefaultSkin == skinId) {
        return;
    }

    if (IsValid(m_DefaultSkin)) {
        m_Vault.release(m_DefaultSkin);
    }

    m_DefaultSkin = skinId;
    if (IsValid(m_DefaultSkin)) {
        m_Vault.retain(m_DefaultSkin);
    }

    refreshAllSprites();
}

ObjectId Stage::spawn(const std::string& name, ResId roleId, ResId skinOverride) {
    if (!IsValid(roleId) || GetKind(roleId) != ResourceKind::Role) {
        return kInvalidObjectId;
    }

    const Role* role = m_Vault.getRole(roleId);
    if (!role) {
        return kInvalidObjectId;
    }

    ObjectId id = m_NextObjectId++;
    const std::size_t index = m_ObjectIds.size();

    m_ObjectIds.push_back(id);
    m_ObjectNames.push_back(name);
    m_ObjectPositions.emplace_back(0.0f, 0.0f);
    m_ObjectScales.emplace_back(1.0f, 1.0f);
    m_ObjectRotations.push_back(0.0f);
    m_ObjectRoles.push_back(roleId);
    m_ObjectSkins.push_back(skinOverride);
    m_ObjectLookup.emplace(id, index);

    m_Vault.retain(roleId);
    if (IsValid(skinOverride)) {
        m_Vault.retain(skinOverride);
    }

    if (role->physics) {
        addPhysics(id, *role->physics);
    }
    if (role->collider) {
        addCollider(id, *role->collider);
    }
    if (role->controls) {
        addControls(id, *role->controls);
    }
    if (role->script) {
        addScript(id, *role->script);
    }
    if (role->sprite) {
        if (auto sprite = resolveSprite(*role, name, skinOverride)) {
            addSprite(id, *sprite);
        }
    }

    EventPayload payload = makePayload(id, id, kInvalidObjectId);
    queueEvent(EventType::Start, payload);

    return id;
}

bool Stage::addPhysics(ObjectId id, const Physics& physics) {
    if (!contains(id)) {
        return false;
    }

    if (auto* existing = m_Physics.get(id); existing) {
        *existing = physics;
        return true;
    }

    return m_Physics.add(id, physics);
}

bool Stage::addSprite(ObjectId id, const Sprite& sprite) {
    if (!contains(id)) {
        return false;
    }

    Sprite copy = sprite;
    retainSpriteResources(copy);

    if (auto* existing = m_Sprites.get(id); existing) {
        releaseSpriteResources(*existing);
        *existing = copy;
        return true;
    }

    if (!m_Sprites.add(id, copy)) {
        releaseSpriteResources(copy);
        return false;
    }
    return true;
}

bool Stage::addCollider(ObjectId id, const Collider& collider) {
    if (!contains(id)) {
        return false;
    }

    if (auto* existing = m_Colliders.get(id); existing) {
        *existing = collider;
        return true;
    }

    return m_Colliders.add(id, collider);
}

bool Stage::addControls(ObjectId id, const Controls& controls) {
    if (!contains(id)) {
        return false;
    }

    if (auto* existing = m_Controls.get(id); existing) {
        *existing = controls;
        return true;
    }

    return m_Controls.add(id, controls);
}

bool Stage::addScript(ObjectId id, const Script& script) {
    if (!contains(id)) {
        return false;
    }

    if (auto* existing = m_Scripts.get(id); existing) {
        *existing = script;
        return true;
    }

    return m_Scripts.add(id, script);
}

bool Stage::removePhysics(ObjectId id) {
    return m_Physics.remove(id);
}

bool Stage::removeSprite(ObjectId id) {
    if (auto* existing = m_Sprites.get(id); existing) {
        releaseSpriteResources(*existing);
        return m_Sprites.remove(id);
    }
    return false;
}

bool Stage::removeCollider(ObjectId id) {
    if (m_Colliders.remove(id)) {
        purgeContacts(id);
        return true;
    }
    return false;
}

bool Stage::removeControls(ObjectId id) {
    return m_Controls.remove(id);
}

bool Stage::removeScript(ObjectId id) {
    return m_Scripts.remove(id);
}

bool Stage::remove(ObjectId id) {
    auto indexOpt = indexFor(id);
    if (!indexOpt) {
        return false;
    }

    removePhysics(id);
    removeCollider(id);
    removeControls(id);
    removeScript(id);
    removeSprite(id);

    clearTimersFor(id);

    releaseResources(id);

    const std::size_t index = *indexOpt;
    const std::size_t lastIndex = m_ObjectIds.size() - 1;

    if (index != lastIndex) {
        m_ObjectIds[index] = m_ObjectIds[lastIndex];
        m_ObjectNames[index] = std::move(m_ObjectNames[lastIndex]);
        m_ObjectPositions[index] = m_ObjectPositions[lastIndex];
        m_ObjectScales[index] = m_ObjectScales[lastIndex];
        m_ObjectRotations[index] = m_ObjectRotations[lastIndex];
        m_ObjectRoles[index] = m_ObjectRoles[lastIndex];
        m_ObjectSkins[index] = m_ObjectSkins[lastIndex];
        m_ObjectLookup[m_ObjectIds[index]] = index;
    }

    m_ObjectIds.pop_back();
    m_ObjectNames.pop_back();
    m_ObjectPositions.pop_back();
    m_ObjectScales.pop_back();
    m_ObjectRotations.pop_back();
    m_ObjectRoles.pop_back();
    m_ObjectSkins.pop_back();
    m_ObjectLookup.erase(id);

    return true;
}

void Stage::clear() {
    while (!m_ObjectIds.empty()) {
        remove(m_ObjectIds.back());
    }
    m_Contacts.clear();
    m_Timers.clear();
    m_EventBus.clear();
}

Vec2 Stage::position(ObjectId id) const {
    auto indexOpt = indexFor(id);
    if (!indexOpt) {
        return Vec2{};
    }
    return m_ObjectPositions[*indexOpt];
}

void Stage::setPosition(ObjectId id, const Vec2& value) {
    auto indexOpt = indexFor(id);
    if (!indexOpt) {
        return;
    }
    m_ObjectPositions[*indexOpt] = value;
}

Vec2 Stage::scale(ObjectId id) const {
    auto indexOpt = indexFor(id);
    if (!indexOpt) {
        return Vec2{ 1.0f, 1.0f };
    }
    return m_ObjectScales[*indexOpt];
}

void Stage::setScale(ObjectId id, const Vec2& value) {
    auto indexOpt = indexFor(id);
    if (!indexOpt) {
        return;
    }
    m_ObjectScales[*indexOpt] = value;
}

float Stage::rotation(ObjectId id) const {
    auto indexOpt = indexFor(id);
    if (!indexOpt) {
        return 0.0f;
    }
    return m_ObjectRotations[*indexOpt];
}

void Stage::setRotation(ObjectId id, float value) {
    auto indexOpt = indexFor(id);
    if (!indexOpt) {
        return;
    }
    m_ObjectRotations[*indexOpt] = value;
}

const std::string& Stage::nameOf(ObjectId id) const {
    static const std::string kEmpty;
    auto indexOpt = indexFor(id);
    if (!indexOpt) {
        return kEmpty;
    }
    return m_ObjectNames[*indexOpt];
}

void Stage::setName(ObjectId id, const std::string& value) {
    auto indexOpt = indexFor(id);
    if (!indexOpt) {
        return;
    }
    m_ObjectNames[*indexOpt] = value;
}

ResId Stage::roleOf(ObjectId id) const {
    auto indexOpt = indexFor(id);
    if (!indexOpt) {
        return kInvalidResId;
    }
    return m_ObjectRoles[*indexOpt];
}

ResId Stage::skinOf(ObjectId id) const {
    auto indexOpt = indexFor(id);
    if (!indexOpt) {
        return kInvalidResId;
    }
    return m_ObjectSkins[*indexOpt];
}

bool Stage::setSkin(ObjectId id, ResId skinId) {
    auto indexOpt = indexFor(id);
    if (!indexOpt) {
        return false;
    }

    ResId& stored = m_ObjectSkins[*indexOpt];
    if (stored == skinId) {
        return true;
    }

    if (IsValid(stored)) {
        m_Vault.release(stored);
    }

    stored = skinId;
    if (IsValid(stored)) {
        m_Vault.retain(stored);
    }

    refreshSpriteFor(id);
    return true;
}

bool Stage::has(ObjectId id, Category category) const {
    if (!contains(id)) {
        return false;
    }

    switch (category) {
    case Category::Physics:
        return m_Physics.contains(id);
    case Category::Collider:
        return m_Colliders.contains(id);
    case Category::Sprite:
        return m_Sprites.contains(id);
    case Category::Controls:
        return m_Controls.contains(id);
    case Category::Script:
        return m_Scripts.contains(id);
    }
    return false;
}

bool Stage::contains(ObjectId id) const {
    return m_ObjectLookup.find(id) != m_ObjectLookup.end();
}

Stage::CapabilitySlice<Physics> Stage::physics() const {
    return { m_Physics.owners, m_Physics.values };
}

Stage::CapabilitySlice<Sprite> Stage::sprites() const {
    return { m_Sprites.owners, m_Sprites.values };
}

Stage::CapabilitySlice<Collider> Stage::colliders() const {
    return { m_Colliders.owners, m_Colliders.values };
}

Stage::CapabilitySlice<Controls> Stage::controls() const {
    return { m_Controls.owners, m_Controls.values };
}

Stage::CapabilitySlice<Script> Stage::scripts() const {
    return { m_Scripts.owners, m_Scripts.values };
}

Physics* Stage::physicsFor(ObjectId id) {
    return m_Physics.get(id);
}

const Physics* Stage::physicsFor(ObjectId id) const {
    return m_Physics.get(id);
}

Sprite* Stage::spriteFor(ObjectId id) {
    return m_Sprites.get(id);
}

const Sprite* Stage::spriteFor(ObjectId id) const {
    return m_Sprites.get(id);
}

Collider* Stage::colliderFor(ObjectId id) {
    return m_Colliders.get(id);
}

const Collider* Stage::colliderFor(ObjectId id) const {
    return m_Colliders.get(id);
}

Controls* Stage::controlsFor(ObjectId id) {
    return m_Controls.get(id);
}

const Controls* Stage::controlsFor(ObjectId id) const {
    return m_Controls.get(id);
}

Script* Stage::scriptFor(ObjectId id) {
    return m_Scripts.get(id);
}

const Script* Stage::scriptFor(ObjectId id) const {
    return m_Scripts.get(id);
}

Object Stage::makeObject(ObjectId id) {
    return Object(*this, id);
}

std::optional<Object> Stage::find(ObjectId id) {
    if (!contains(id)) {
        return std::nullopt;
    }
    return Object(*this, id);
}

std::uint32_t Stage::addPhaseCallback(StagePhase phase, PhaseCallback callback) {
    const std::size_t index = static_cast<std::size_t>(phase);
    const std::uint32_t handle = m_NextPhaseHandle++;
    m_PhaseCallbacks[index].push_back({ handle, std::move(callback) });
    return handle;
}

void Stage::removePhaseCallback(StagePhase phase, std::uint32_t handle) {
    const std::size_t index = static_cast<std::size_t>(phase);
    auto& list = m_PhaseCallbacks[index];
    auto it = std::remove_if(list.begin(), list.end(), [handle](const PhaseEntry& entry) {
        return entry.handle == handle;
    });
    list.erase(it, list.end());
}

void Stage::update(float deltaTime) {
    m_EventBus.process(*this);

    m_Time += deltaTime;

    runPhaseHandlers(StagePhase::Input, deltaTime);

    updateTimers(deltaTime);
    runPhaseHandlers(StagePhase::Timers, deltaTime);

    runPhaseHandlers(StagePhase::ScriptPre, deltaTime);
    runScriptPhase(true, deltaTime);

    m_FixedAccumulator += deltaTime;
    int steps = 0;
    while (m_FixedAccumulator >= m_FixedDelta && steps < kMaxPhysicsStepsPerFrame) {
        integratePhysics(m_FixedDelta);
        runPhaseHandlers(StagePhase::Physics, m_FixedDelta);
        m_FixedAccumulator -= m_FixedDelta;
        ++steps;
    }
    if (steps == kMaxPhysicsStepsPerFrame) {
        m_FixedAccumulator = 0.0f;
    }

    resolveCollisions(deltaTime);
    m_EventBus.process(*this);

    runPhaseHandlers(StagePhase::Collision, deltaTime);
    runScriptPhase(false, deltaTime);
    runPhaseHandlers(StagePhase::ScriptPost, deltaTime);
    runPhaseHandlers(StagePhase::Culling, deltaTime);
    runPhaseHandlers(StagePhase::Render, deltaTime);

    queueTickEvents(deltaTime);
    m_EventBus.process(*this);
}

std::optional<std::size_t> Stage::indexFor(ObjectId id) const {
    auto it = m_ObjectLookup.find(id);
    if (it == m_ObjectLookup.end()) {
        return std::nullopt;
    }
    return it->second;
}

void Stage::releaseResources(ObjectId id, bool releaseRole, bool releaseSkin) {
    auto indexOpt = indexFor(id);
    if (!indexOpt) {
        return;
    }
    const std::size_t index = *indexOpt;

    if (releaseRole) {
        ResId roleId = m_ObjectRoles[index];
        if (IsValid(roleId)) {
            m_Vault.release(roleId);
        }
    }

    if (releaseSkin) {
        ResId skinId = m_ObjectSkins[index];
        if (IsValid(skinId)) {
            m_Vault.release(skinId);
        }
    }
}

void Stage::refreshSpriteFor(ObjectId id) {
    auto indexOpt = indexFor(id);
    if (!indexOpt) {
        return;
    }

    const std::size_t index = *indexOpt;
    const ResId roleId = m_ObjectRoles[index];
    const Role* role = m_Vault.getRole(roleId);
    if (!role || !role->sprite) {
        removeSprite(id);
        return;
    }

    const std::string& objName = m_ObjectNames[index];
    const ResId objectSkin = m_ObjectSkins[index];
    auto sprite = resolveSprite(*role, objName, objectSkin);
    if (!sprite) {
        removeSprite(id);
        return;
    }

    addSprite(id, *sprite);
}

void Stage::refreshAllSprites() {
    for (ObjectId id : m_ObjectIds) {
        refreshSpriteFor(id);
    }
}

std::optional<Sprite> Stage::resolveSprite(const Role& role, const std::string& objectName, ResId objectSkin) {
    if (!role.sprite) {
        return std::nullopt;
    }

    Sprite sprite = *role.sprite;

    auto applySkin = [&](ResId skinId) {
        if (!IsValid(skinId)) {
            return;
        }
        if (const Skin* skin = m_Vault.getSkin(skinId); skin) {
            const std::string roleKey = normalizeKey(role.name);
            const std::string objectKey = normalizeKey(objectName);

            auto resolveOverride = [&](const auto& map) -> const std::string* {
                if (!objectKey.empty()) {
                    if (auto it = map.find(objectKey); it != map.end()) {
                        return &it->second;
                    }
                }
                if (!roleKey.empty()) {
                    if (auto it = map.find(roleKey); it != map.end()) {
                        return &it->second;
                    }
                }
                return nullptr;
            };

            if (const std::string* imageOverride = resolveOverride(skin->imageOverrides)) {
                sprite.image = m_Vault.image(*imageOverride);
            }
            if (const std::string* animOverride = resolveOverride(skin->animationOverrides)) {
                sprite.animation = m_Vault.animation(*animOverride);
            }
        }
    };

    applySkin(m_DefaultSkin);
    applySkin(objectSkin);

    return sprite;
}

void Stage::retainSpriteResources(const Sprite& sprite) {
    if (IsValid(sprite.image)) {
        m_Vault.retain(sprite.image);
    }
    if (IsValid(sprite.animation)) {
        m_Vault.retain(sprite.animation);
    }
}

void Stage::releaseSpriteResources(const Sprite& sprite) {
    if (IsValid(sprite.image)) {
        m_Vault.release(sprite.image);
    }
    if (IsValid(sprite.animation)) {
        m_Vault.release(sprite.animation);
    }
}

std::string Stage::normalizeKey(const std::string& value) {
    return NormalizeKey(value);
}

void Stage::runPhaseHandlers(StagePhase phase, float deltaTime) {
    const std::size_t index = static_cast<std::size_t>(phase);
    auto& entries = m_PhaseCallbacks[index];
    for (const auto& entry : entries) {
        if (entry.callback) {
            entry.callback(*this, deltaTime);
        }
    }
}

void Stage::runScriptPhase(bool prePhase, float deltaTime) {
    for (std::size_t i = 0; i < m_Scripts.values.size(); ++i) {
        const ObjectId id = m_Scripts.owners[i];
        if (!contains(id)) {
            continue;
        }
        Script& script = m_Scripts.values[i];
        Script::UpdateFn fn = nullptr;
        if (prePhase) {
            fn = script.preUpdate ? script.preUpdate : script.update;
        }
        else {
            fn = script.postUpdate ? script.postUpdate : script.update;
        }
        if (fn) {
            Object object(*this, id);
            fn(object, deltaTime);
        }
    }
}

void Stage::integratePhysics(float fixedDelta) {
    if (fixedDelta <= 0.0f || !std::isfinite(fixedDelta)) {
        return;
    }

    for (std::size_t i = 0; i < m_Physics.values.size(); ++i) {
        const ObjectId id = m_Physics.owners[i];
        if (!contains(id)) {
            continue;
        }
        auto indexOpt = indexFor(id);
        if (!indexOpt) {
            continue;
        }
        Physics& physics = m_Physics.values[i];
        if (physics.kinematic) {
            physics.acceleration = Vec2{};
            continue;
        }

        Vec2 totalAcceleration = physics.acceleration;
        totalAcceleration.x += m_Gravity.x * physics.gravityScale;
        totalAcceleration.y += m_Gravity.y * physics.gravityScale;

        physics.velocity += totalAcceleration * fixedDelta;
        if (physics.drag > 0.0f) {
            const float dragFactor = std::max(0.0f, 1.0f - physics.drag * fixedDelta);
            physics.velocity = physics.velocity * dragFactor;
        }

        m_ObjectPositions[*indexOpt] += physics.velocity * fixedDelta;
        physics.acceleration = Vec2{};
    }
}

void Stage::queueEvent(EventType type, const EventPayload& payload) {
    m_EventBus.queue(type, payload);
}

void Stage::queueUse(ObjectId sender, ObjectId target, std::uint32_t tag) {
    if (!contains(sender)) {
        return;
    }
    EventPayload payload = makePayload(sender, sender, target, 0.0f, 0.0f, tag);
    queueEvent(EventType::Use, payload);
}

std::uint32_t Stage::addTimer(ObjectId target, float duration, bool repeat, std::uint32_t tag) {
    if (!(duration > 0.0f) || !std::isfinite(duration)) {
        return 0;
    }

    StageTimer timer;
    timer.id = m_NextTimerId++;
    timer.target = target;
    timer.duration = duration;
    timer.remaining = duration;
    timer.repeat = repeat;
    timer.tag = tag;
    m_Timers.push_back(timer);
    return timer.id;
}

bool Stage::cancelTimer(std::uint32_t timerId) {
    if (timerId == 0) {
        return false;
    }
    for (std::size_t i = 0; i < m_Timers.size(); ++i) {
        if (m_Timers[i].id == timerId) {
            m_Timers[i] = m_Timers.back();
            m_Timers.pop_back();
            return true;
        }
    }
    return false;
}

void Stage::queueTickEvents(float deltaTime) {
    for (ObjectId id : m_ObjectIds) {
        EventPayload payload = makePayload(id, id, kInvalidObjectId, deltaTime, 0.0f, 0);
        queueEvent(EventType::Tick, payload);
    }
}

void Stage::updateTimers(float deltaTime) {
    if (m_Timers.empty()) {
        return;
    }

    for (std::size_t i = 0; i < m_Timers.size();) {
        StageTimer& timer = m_Timers[i];
        timer.remaining -= deltaTime;
        bool triggerTimer = timer.remaining <= 0.0f;
        if (triggerTimer) {
            EventPayload payload = makePayload(timer.target, timer.target, kInvalidObjectId, timer.duration, deltaTime, timer.tag);
            queueEvent(EventType::Timer, payload);

            if (timer.repeat) {
                timer.remaining += timer.duration;
                if (timer.remaining < 0.0f) {
                    timer.remaining = timer.duration;
                }
                ++i;
            }
            else {
                m_Timers[i] = m_Timers.back();
                m_Timers.pop_back();
            }
        }
        else {
            ++i;
        }
    }
}

void Stage::resolveCollisions(float deltaTime) {
    for (auto& entry : m_Contacts) {
        entry.second.active = false;
    }

    struct ColliderView {
        ObjectId id;
        float minX;
        float minY;
        float maxX;
        float maxY;
        bool trigger;
        std::uint32_t layer;
        std::uint32_t mask;
    };

    std::vector<ColliderView> colliders;
    colliders.reserve(m_Colliders.values.size());

    for (std::size_t i = 0; i < m_Colliders.values.size(); ++i) {
        const ObjectId id = m_Colliders.owners[i];
        if (!contains(id)) {
            continue;
        }
        auto indexOpt = indexFor(id);
        if (!indexOpt) {
            continue;
        }

        const Collider& collider = m_Colliders.values[i];
        const Vec2& position = m_ObjectPositions[*indexOpt];
        const Vec2& scale = m_ObjectScales[*indexOpt];

        const float scaleX = std::abs(scale.x);
        const float scaleY = std::abs(scale.y);
        const float width = collider.w * scaleX;
        const float height = collider.h * scaleY;
        if (!(width > 0.0f) || !(height > 0.0f)) {
            continue;
        }

        const float offsetX = collider.x * scaleX;
        const float offsetY = collider.y * scaleY;

        ColliderView view{};
        view.id = id;
        view.minX = position.x + offsetX;
        view.minY = position.y + offsetY;
        view.maxX = view.minX + width;
        view.maxY = view.minY + height;
        view.trigger = collider.trigger;
        view.layer = collider.layer;
        view.mask = collider.mask;

        colliders.push_back(view);
    }

    auto makeKey = [](ObjectId lhs, ObjectId rhs) {
        ContactKey key{ lhs, rhs };
        if (key.a > key.b) {
            std::swap(key.a, key.b);
        }
        return key;
    };

    for (std::size_t i = 0; i < colliders.size(); ++i) {
        const ColliderView& a = colliders[i];
        for (std::size_t j = i + 1; j < colliders.size(); ++j) {
            const ColliderView& b = colliders[j];

            if ((a.mask & b.layer) == 0 || (b.mask & a.layer) == 0) {
                continue;
            }

            const bool overlap = (a.minX <= b.maxX && a.maxX >= b.minX &&
                                   a.minY <= b.maxY && a.maxY >= b.minY);

            const ContactKey key = makeKey(a.id, b.id);
            auto it = m_Contacts.find(key);

            if (overlap) {
                const bool triggerPair = a.trigger || b.trigger;
                if (it == m_Contacts.end()) {
                    ContactState state;
                    state.trigger = triggerPair;
                    state.active = true;
                    it = m_Contacts.emplace(key, state).first;

                    EventPayload enterA = makePayload(a.id, a.id, b.id, 0.0f, deltaTime, 0);
                    EventPayload enterB = makePayload(b.id, b.id, a.id, 0.0f, deltaTime, 0);
                    queueEvent(EventType::Enter, enterA);
                    queueEvent(EventType::Enter, enterB);

                    if (!triggerPair) {
                        EventPayload hitA = makePayload(a.id, a.id, b.id, deltaTime, 0.0f, 0);
                        EventPayload hitB = makePayload(b.id, b.id, a.id, deltaTime, 0.0f, 0);
                        queueEvent(EventType::Hit, hitA);
                        queueEvent(EventType::Hit, hitB);
                    }
                }
                else {
                    it->second.trigger = triggerPair;
                    it->second.active = true;
                    if (!triggerPair) {
                        EventPayload hitA = makePayload(a.id, a.id, b.id, deltaTime, 0.0f, 0);
                        EventPayload hitB = makePayload(b.id, b.id, a.id, deltaTime, 0.0f, 0);
                        queueEvent(EventType::Hit, hitA);
                        queueEvent(EventType::Hit, hitB);
                    }
                }
            }
            else if (it != m_Contacts.end()) {
                EventPayload exitA = makePayload(a.id, a.id, b.id, 0.0f, deltaTime, 0);
                EventPayload exitB = makePayload(b.id, b.id, a.id, 0.0f, deltaTime, 0);
                queueEvent(EventType::Exit, exitA);
                queueEvent(EventType::Exit, exitB);
                m_Contacts.erase(it);
            }
        }
    }

    for (auto it = m_Contacts.begin(); it != m_Contacts.end();) {
        ContactState& state = it->second;
        if (!state.active) {
            const ContactKey& key = it->first;
            EventPayload exitA = makePayload(key.a, key.a, key.b, 0.0f, deltaTime, 0);
            EventPayload exitB = makePayload(key.b, key.b, key.a, 0.0f, deltaTime, 0);
            queueEvent(EventType::Exit, exitA);
            queueEvent(EventType::Exit, exitB);
            it = m_Contacts.erase(it);
        }
        else {
            state.active = false;
            ++it;
        }
    }
}

void Stage::purgeContacts(ObjectId id) {
    if (m_Contacts.empty()) {
        return;
    }

    for (auto it = m_Contacts.begin(); it != m_Contacts.end();) {
        if (it->first.a == id || it->first.b == id) {
            const ObjectId other = (it->first.a == id) ? it->first.b : it->first.a;
            EventPayload exitOther = makePayload(other, other, id, 0.0f, 0.0f, 0);
            queueEvent(EventType::Exit, exitOther);
            it = m_Contacts.erase(it);
        }
        else {
            ++it;
        }
    }
}

void Stage::clearTimersFor(ObjectId id) {
    if (m_Timers.empty()) {
        return;
    }

    for (std::size_t i = 0; i < m_Timers.size();) {
        if (m_Timers[i].target == id) {
            m_Timers[i] = m_Timers.back();
            m_Timers.pop_back();
        }
        else {
            ++i;
        }
    }
}

EventPayload Stage::makePayload(ObjectId sender, ObjectId target, ObjectId other, float value, float aux, std::uint32_t data) {
    EventPayload payload{};
    payload.sender = sender;
    payload.target = target;
    payload.other = other;
    payload.value = value;
    payload.aux = aux;
    payload.data = data;
    return payload;
}

} // namespace sage2d
