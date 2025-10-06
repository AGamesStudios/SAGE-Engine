#pragma once

#include "Types.h"
#include "Capabilities.h"
#include "ResourceId.h"
#include "Role.h"
#include "Skin.h"
#include "Vault.h"
#include "EventBus.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef Stage
#error "Stage macro is defined; rename or undefine before including Stage.h"
#endif

namespace sage2d {

    class Stage;
    class StageManager;

    [[nodiscard]] bool EventTargetsObject(ObjectId objectId, const Event& event);

    enum class Category {
        Physics,
        Collider,
        Sprite,
        Controls,
        Script
    };

    enum class StagePhase {
        Input = 0,
        Timers,
        ScriptPre,
        Physics,
        Collision,
        ScriptPost,
        Culling,
        Render,
        Count
    };

    struct Object {
        Object() = default;
        Object(Stage& stage, ObjectId id);

        using EventHandle = EventBus::ListenerId;
        using EventHandler = std::function<void(Object&, const Event&)>;

        [[nodiscard]] bool valid() const;
        [[nodiscard]] ObjectId id() const { return m_Id; }

        [[nodiscard]] Vec2 position() const;
        void setPosition(const Vec2& value);

        [[nodiscard]] Vec2 scale() const;
        void setScale(const Vec2& value);

        [[nodiscard]] float rotation() const;
        void setRotation(float value);

        [[nodiscard]] const std::string& name() const;
        void setName(const std::string& value);

        [[nodiscard]] ResId role() const;
        [[nodiscard]] ResId skin() const;
        bool setSkin(ResId skinId);

        [[nodiscard]] bool has(Category category) const;

        Physics* physics();
        Sprite* sprite();
        Collider* collider();
        Controls* controls();
        Script* script();

        EventHandle on(EventType type, EventHandler handler);
        void off(EventType type, EventHandle handle);

        void use(ObjectId target = kInvalidObjectId, std::uint32_t tag = 0) const;
        std::uint32_t addTimer(float duration, bool repeat = false, std::uint32_t tag = 0) const;
        bool cancelTimer(std::uint32_t timerId) const;
    private:
        Stage* m_Stage{ nullptr };
        ObjectId m_Id{ kInvalidObjectId };
    };

    class Stage {
    public:
        using PhaseCallback = std::function<void(Stage&, float)>;

        template<typename T>
        struct CapabilitySlice {
            const std::vector<ObjectId>& owners;
            const std::vector<T>& values;
        };

        Stage(std::string name, Vault& vault);
        ~Stage();

        Stage(const Stage&) = delete;
        Stage& operator=(const Stage&) = delete;
        Stage(Stage&&) = default;
        Stage& operator=(Stage&&) = default;

        [[nodiscard]] const std::string& name() const { return m_Name; }
        [[nodiscard]] Vault& vault() { return m_Vault; }
        [[nodiscard]] const Vault& vault() const { return m_Vault; }

        void setStageManager(StageManager* manager);
        [[nodiscard]] StageManager* manager() const { return m_Manager; }

        void setOnEnter(std::function<void(Stage&)> callback);
        void setOnExit(std::function<void(Stage&)> callback);

        void onEnter();
        void onExit();

        void setFixedDelta(float stepSeconds);
        [[nodiscard]] float fixedDelta() const { return m_FixedDelta; }

        void setGravity(Vec2 gravity);
        [[nodiscard]] Vec2 gravity() const { return m_Gravity; }

        void setDefaultSkin(ResId skinId);
        [[nodiscard]] ResId defaultSkin() const { return m_DefaultSkin; }

        ObjectId spawn(const std::string& name, ResId roleId, ResId skinOverride = kInvalidResId);
        bool addPhysics(ObjectId id, const Physics& physics);
        bool addSprite(ObjectId id, const Sprite& sprite);
        bool addCollider(ObjectId id, const Collider& collider);
        bool addControls(ObjectId id, const Controls& controls);
        bool addScript(ObjectId id, const Script& script);

        bool removePhysics(ObjectId id);
        bool removeSprite(ObjectId id);
        bool removeCollider(ObjectId id);
        bool removeControls(ObjectId id);
        bool removeScript(ObjectId id);

        bool remove(ObjectId id);
        void clear();

    EventBus& events() { return m_EventBus; }
    const EventBus& events() const { return m_EventBus; }

    void queueEvent(EventType type, const EventPayload& payload);
    void queueUse(ObjectId sender, ObjectId target = kInvalidObjectId, std::uint32_t tag = 0);
    std::uint32_t addTimer(ObjectId target, float duration, bool repeat = false, std::uint32_t tag = 0);
    bool cancelTimer(std::uint32_t timerId);

        [[nodiscard]] std::size_t objectCount() const { return m_ObjectIds.size(); }

        [[nodiscard]] Vec2 position(ObjectId id) const;
        void setPosition(ObjectId id, const Vec2& value);

        [[nodiscard]] Vec2 scale(ObjectId id) const;
        void setScale(ObjectId id, const Vec2& value);

        [[nodiscard]] float rotation(ObjectId id) const;
        void setRotation(ObjectId id, float value);

        [[nodiscard]] const std::string& nameOf(ObjectId id) const;
        void setName(ObjectId id, const std::string& value);

        [[nodiscard]] ResId roleOf(ObjectId id) const;
        [[nodiscard]] ResId skinOf(ObjectId id) const;
        bool setSkin(ObjectId id, ResId skinId);

        [[nodiscard]] bool has(ObjectId id, Category category) const;

    [[nodiscard]] bool contains(ObjectId id) const;

        [[nodiscard]] CapabilitySlice<Physics> physics() const;
        [[nodiscard]] CapabilitySlice<Sprite> sprites() const;
        [[nodiscard]] CapabilitySlice<Collider> colliders() const;
        [[nodiscard]] CapabilitySlice<Controls> controls() const;
        [[nodiscard]] CapabilitySlice<Script> scripts() const;

    [[nodiscard]] Physics* physicsFor(ObjectId id);
    [[nodiscard]] const Physics* physicsFor(ObjectId id) const;
    [[nodiscard]] Sprite* spriteFor(ObjectId id);
    [[nodiscard]] const Sprite* spriteFor(ObjectId id) const;
    [[nodiscard]] Collider* colliderFor(ObjectId id);
    [[nodiscard]] const Collider* colliderFor(ObjectId id) const;
    [[nodiscard]] Controls* controlsFor(ObjectId id);
    [[nodiscard]] const Controls* controlsFor(ObjectId id) const;
    [[nodiscard]] Script* scriptFor(ObjectId id);
    [[nodiscard]] const Script* scriptFor(ObjectId id) const;

        [[nodiscard]] Object makeObject(ObjectId id);
        [[nodiscard]] std::optional<Object> find(ObjectId id);

        std::uint32_t addPhaseCallback(StagePhase phase, PhaseCallback callback);
        void removePhaseCallback(StagePhase phase, std::uint32_t handle);

        void update(float deltaTime);

        [[nodiscard]] float elapsedTime() const { return m_Time; }

    private:
        template<typename CapabilityT>
        struct CapabilityStorage {
            std::vector<ObjectId> owners;
            std::vector<CapabilityT> values;
            std::unordered_map<ObjectId, std::size_t> lookup;

            bool contains(ObjectId id) const {
                return lookup.find(id) != lookup.end();
            }

            CapabilityT* get(ObjectId id) {
                auto it = lookup.find(id);
                if (it == lookup.end()) {
                    return nullptr;
                }
                return &values[it->second];
            }

            const CapabilityT* get(ObjectId id) const {
                auto it = lookup.find(id);
                if (it == lookup.end()) {
                    return nullptr;
                }
                return &values[it->second];
            }

            bool add(ObjectId id, CapabilityT capability) {
                if (contains(id)) {
                    return false;
                }
                const std::size_t index = values.size();
                owners.push_back(id);
                values.push_back(std::move(capability));
                lookup.emplace(id, index);
                return true;
            }

            bool remove(ObjectId id) {
                auto it = lookup.find(id);
                if (it == lookup.end()) {
                    return false;
                }
                const std::size_t index = it->second;
                const std::size_t lastIndex = values.size() - 1;
                if (index != lastIndex) {
                    owners[index] = owners[lastIndex];
                    values[index] = std::move(values[lastIndex]);
                    lookup[owners[index]] = index;
                }
                owners.pop_back();
                values.pop_back();
                lookup.erase(it);
                return true;
            }

            void clear() {
                owners.clear();
                values.clear();
                lookup.clear();
            }
        };

        struct PhaseEntry {
            std::uint32_t handle{ 0 };
            PhaseCallback callback;
        };

        [[nodiscard]] std::optional<std::size_t> indexFor(ObjectId id) const;
        void releaseResources(ObjectId id, bool releaseRole = true, bool releaseSkin = true);
    void refreshSpriteFor(ObjectId id);
    void refreshAllSprites();
    std::optional<Sprite> resolveSprite(const Role& role, const std::string& objectName, ResId objectSkin);
        void retainSpriteResources(const Sprite& sprite);
        void releaseSpriteResources(const Sprite& sprite);
        static std::string normalizeKey(const std::string& value);

        void runPhaseHandlers(StagePhase phase, float deltaTime);
        void runScriptPhase(bool prePhase, float deltaTime);
        void integratePhysics(float fixedDelta);
    void queueTickEvents(float deltaTime);
    void updateTimers(float deltaTime);
    void resolveCollisions(float deltaTime);
    void purgeContacts(ObjectId id);
    void clearTimersFor(ObjectId id);
    static EventPayload makePayload(ObjectId sender, ObjectId target, ObjectId other, float value = 0.0f, float aux = 0.0f, std::uint32_t data = 0);

        std::string m_Name;
        Vault& m_Vault;
        StageManager* m_Manager{ nullptr };

        std::vector<ObjectId> m_ObjectIds;
        std::vector<std::string> m_ObjectNames;
        std::vector<Vec2> m_ObjectPositions;
        std::vector<Vec2> m_ObjectScales;
        std::vector<float> m_ObjectRotations;
        std::vector<ResId> m_ObjectRoles;
        std::vector<ResId> m_ObjectSkins;
        std::unordered_map<ObjectId, std::size_t> m_ObjectLookup;
        ObjectId m_NextObjectId{ 1 };

        CapabilityStorage<Physics> m_Physics;
        CapabilityStorage<Sprite> m_Sprites;
        CapabilityStorage<Collider> m_Colliders;
        CapabilityStorage<Controls> m_Controls;
        CapabilityStorage<Script> m_Scripts;

        std::array<std::vector<PhaseEntry>, static_cast<std::size_t>(StagePhase::Count)> m_PhaseCallbacks{};
        std::uint32_t m_NextPhaseHandle{ 1 };

        struct ContactKey {
            ObjectId a{ kInvalidObjectId };
            ObjectId b{ kInvalidObjectId };

            bool operator==(const ContactKey&) const = default;
        };

        struct ContactKeyHash {
            std::size_t operator()(const ContactKey& key) const noexcept {
                const std::uint64_t high = static_cast<std::uint64_t>(key.a);
                const std::uint64_t low = static_cast<std::uint64_t>(key.b);
                return static_cast<std::size_t>((high << 32) ^ low);
            }
        };

        struct ContactState {
            bool trigger{ false };
            bool active{ false };
        };

        struct StageTimer {
            std::uint32_t id{ 0 };
            ObjectId target{ kInvalidObjectId };
            float duration{ 0.0f };
            float remaining{ 0.0f };
            bool repeat{ false };
            std::uint32_t tag{ 0 };
        };

        float m_Time{ 0.0f };
        float m_FixedAccumulator{ 0.0f };
        float m_FixedDelta{ 1.0f / 60.0f };
        Vec2 m_Gravity{ 0.0f, -9.81f };

        ResId m_DefaultSkin{ kInvalidResId };

        std::function<void(Stage&)> m_OnEnter;
        std::function<void(Stage&)> m_OnExit;

        EventBus m_EventBus;
        std::unordered_map<ContactKey, ContactState, ContactKeyHash> m_Contacts;
        std::vector<StageTimer> m_Timers;
        std::uint32_t m_NextTimerId{ 1 };
    };

    inline Object::Object(Stage& stage, ObjectId id)
        : m_Stage(&stage)
        , m_Id(id) {}

    inline bool Object::valid() const {
        return m_Stage != nullptr && m_Stage->contains(m_Id);
    }

    inline Vec2 Object::position() const {
        return m_Stage ? m_Stage->position(m_Id) : Vec2{};
    }

    inline void Object::setPosition(const Vec2& value) {
        if (m_Stage) {
            m_Stage->setPosition(m_Id, value);
        }
    }

    inline Vec2 Object::scale() const {
        return m_Stage ? m_Stage->scale(m_Id) : Vec2{ 1.0f, 1.0f };
    }

    inline void Object::setScale(const Vec2& value) {
        if (m_Stage) {
            m_Stage->setScale(m_Id, value);
        }
    }

    inline float Object::rotation() const {
        return m_Stage ? m_Stage->rotation(m_Id) : 0.0f;
    }

    inline void Object::setRotation(float value) {
        if (m_Stage) {
            m_Stage->setRotation(m_Id, value);
        }
    }

    inline const std::string& Object::name() const {
        static const std::string kEmpty;
        return m_Stage ? m_Stage->nameOf(m_Id) : kEmpty;
    }

    inline void Object::setName(const std::string& value) {
        if (m_Stage) {
            m_Stage->setName(m_Id, value);
        }
    }

    inline ResId Object::role() const {
        return m_Stage ? m_Stage->roleOf(m_Id) : kInvalidResId;
    }

    inline ResId Object::skin() const {
        return m_Stage ? m_Stage->skinOf(m_Id) : kInvalidResId;
    }

    inline bool Object::setSkin(ResId skinId) {
        return m_Stage ? m_Stage->setSkin(m_Id, skinId) : false;
    }

    inline bool Object::has(Category category) const {
        return m_Stage ? m_Stage->has(m_Id, category) : false;
    }

    inline Physics* Object::physics() {
        return m_Stage ? m_Stage->physicsFor(m_Id) : nullptr;
    }

    inline Sprite* Object::sprite() {
        return m_Stage ? m_Stage->spriteFor(m_Id) : nullptr;
    }

    inline Collider* Object::collider() {
        return m_Stage ? m_Stage->colliderFor(m_Id) : nullptr;
    }

    inline Controls* Object::controls() {
        return m_Stage ? m_Stage->controlsFor(m_Id) : nullptr;
    }

    inline Script* Object::script() {
        return m_Stage ? m_Stage->scriptFor(m_Id) : nullptr;
    }

    inline Object::EventHandle Object::on(EventType type, EventHandler handler) {
        if (!m_Stage || !handler) {
            return 0;
        }

        const ObjectId objectId = m_Id;
        return m_Stage->events().subscribe(type, [objectId, handler = std::move(handler)](Stage& stage, const Event& event) mutable {
            if (!EventTargetsObject(objectId, event)) {
                return;
            }

            Object object(stage, objectId);
            if (!object.valid()) {
                return;
            }

            handler(object, event);
        });
    }

    inline void Object::off(EventType type, EventHandle handle) {
        if (!m_Stage || handle == 0) {
            return;
        }
        m_Stage->events().unsubscribe(type, handle);
    }

    inline void Object::use(ObjectId target, std::uint32_t tag) const {
        if (!m_Stage) {
            return;
        }
        m_Stage->queueUse(m_Id, target, tag);
    }

    inline std::uint32_t Object::addTimer(float duration, bool repeat, std::uint32_t tag) const {
        if (!m_Stage) {
            return 0;
        }
        return m_Stage->addTimer(m_Id, duration, repeat, tag);
    }

    inline bool Object::cancelTimer(std::uint32_t timerId) const {
        if (!m_Stage || timerId == 0) {
            return false;
        }
        return m_Stage->cancelTimer(timerId);
    }

} // namespace sage2d
