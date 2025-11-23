#include "SAGE/Core/ECSSystems.h"
#include "SAGE/Core/ECSGame.h"
#include "SAGE/Graphics/Renderer.h"
#include "SAGE/Graphics/Camera2D.h"
#include "SAGE/Graphics/Tilemap.h"
#include "SAGE/Input/Input.h"
#include "SAGE/Log.h"
#include "SAGE/Physics/PhysicsCommon.h"
#include "SAGE/Core/Scene.h"
#include "SAGE/Scripting/ScriptableEntity.h"

namespace SAGE::ECS {

PhysicsSystem::PhysicsSystem(Physics::PhysicsWorld& world) : m_World(world) {
    m_World.SetBeginContactCallback([this](const Physics::ContactEvent& event) {
        this->OnContact(event);
    });
    m_World.SetEndContactCallback([this](const Physics::ContactEvent& event) {
        this->OnContact(event);
    });
}

void PhysicsSystem::Tick(Registry& reg, float deltaTime) {
    m_CurrentRegistry = &reg;

    // 1. Initialize new bodies and sync Transform -> Body for Kinematic/Static
    reg.ForEach<RigidBodyComponent, TransformComponent>([this, &reg](Entity e, RigidBodyComponent& rb, TransformComponent& trans) {
        PhysicsColliderComponent* collider = reg.Get<PhysicsColliderComponent>(e);
        
        if (!rb.IsValid()) {
            InitBody(e, rb, trans, collider);
        } else {
            // Sync Transform -> Body
            // For Static/Kinematic: Always sync
            // For Dynamic: Sync ONLY if transform changed significantly (teleport)
            bool isDynamic = (rb.type == BodyType::Dynamic);
            
            // Use epsilon for comparison to prevent jitter
            const float kEpsilon = 0.0001f;
            bool posChanged = std::abs(trans.position.x - rb.lastSyncedPosition.x) > kEpsilon || 
                              std::abs(trans.position.y - rb.lastSyncedPosition.y) > kEpsilon;
            bool rotChanged = std::abs(trans.rotation - rb.lastSyncedRotation) > kEpsilon;
            bool transformChanged = posChanged || rotChanged;
            
            if (!isDynamic || transformChanged) {
                SyncTransformToBody(e, rb, trans);
            }
        }
    });

    // 2. Step Physics - Handled in Scene::OnFixedUpdate
    // m_World.Step(deltaTime);

    // 3. Sync Body -> Transform for Dynamic
    reg.ForEach<RigidBodyComponent, TransformComponent>([this, &reg](Entity e, RigidBodyComponent& rb, TransformComponent& trans) {
        if (rb.IsValid() && rb.type == BodyType::Dynamic) {
            SyncBodyToTransform(e, rb, trans);

            // Sync Velocity Body -> Component
            if (auto* vel = reg.Get<VelocityComponent>(e)) {
                Vector2 v = m_World.GetLinearVelocity(rb.bodyHandle);
                float av = m_World.GetAngularVelocity(rb.bodyHandle);
                vel->velocity = v;
                vel->angularVelocity = av;
            }
        }
    });
}

void PhysicsSystem::FixedTick(Registry& reg, float fixedDeltaTime) {
    m_CurrentRegistry = &reg;

    // 1. Sync Transform -> Body (Ensure body is up to date before step)
    reg.ForEach<RigidBodyComponent, TransformComponent>([this, &reg](Entity e, RigidBodyComponent& rb, TransformComponent& trans) {
        PhysicsColliderComponent* collider = reg.Get<PhysicsColliderComponent>(e);
        
        if (!rb.IsValid()) {
            InitBody(e, rb, trans, collider);
        } else {
            // Sync Transform -> Body
            bool isDynamic = (rb.type == BodyType::Dynamic);
            
            const float kEpsilon = 0.0001f;
            bool posChanged = std::abs(trans.position.x - rb.lastSyncedPosition.x) > kEpsilon || 
                              std::abs(trans.position.y - rb.lastSyncedPosition.y) > kEpsilon;
            bool rotChanged = std::abs(trans.rotation - rb.lastSyncedRotation) > kEpsilon;
            bool transformChanged = posChanged || rotChanged;
            
            if (!isDynamic || transformChanged) {
                SyncTransformToBody(e, rb, trans);
            }
        }
    });

    // 2. Apply VelocityComponent -> Body Velocity
    reg.ForEach<RigidBodyComponent, VelocityComponent>([this](Entity, RigidBodyComponent& rb, VelocityComponent& vel) {
        if (rb.IsValid() && rb.type == BodyType::Dynamic) {
            m_World.SetLinearVelocity(rb.bodyHandle, vel.velocity);
            m_World.SetAngularVelocity(rb.bodyHandle, vel.angularVelocity);
        }
    });
}

void PhysicsSystem::InitBody(Entity e, RigidBodyComponent& rb, TransformComponent& trans, PhysicsColliderComponent* collider) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = Physics::ToB2Vec2(trans.position);
    bodyDef.rotation = b2MakeRot(trans.rotation * 0.0174533f); // Rotation in radians
    bodyDef.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(e));
    
    switch (rb.type) {
        case BodyType::Static: bodyDef.type = b2_staticBody; break;
        case BodyType::Kinematic: bodyDef.type = b2_kinematicBody; break;
        case BodyType::Dynamic: bodyDef.type = b2_dynamicBody; break;
    }
    
    bodyDef.motionLocks.angularZ = rb.fixedRotation;
    bodyDef.gravityScale = rb.gravityScale;
    bodyDef.isAwake = rb.awake;

    rb.bodyHandle = m_World.CreateBody(bodyDef);
    
    // Initialize synced state
    rb.lastSyncedPosition = trans.position;
    rb.lastSyncedRotation = trans.rotation;
    
    if (collider && rb.IsValid()) {
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = collider->material.density;
        shapeDef.material.friction = collider->material.friction;
        shapeDef.material.restitution = collider->material.restitution;
        shapeDef.isSensor = collider->isSensor;
        
        b2BodyId bodyId = Physics::ToB2BodyId(rb.bodyHandle);
        
        if (collider->shape == ColliderShape::Box) {
            b2Polygon box = b2MakeBox(collider->size.x * 0.5f * trans.scale.x, collider->size.y * 0.5f * trans.scale.y);
            // Apply offset
            if (collider->offset.x != 0 || collider->offset.y != 0) {
                box.centroid = Physics::ToB2Vec2(collider->offset);
            }
            b2CreatePolygonShape(bodyId, &shapeDef, &box);
        } else if (collider->shape == ColliderShape::Circle) {
            b2Circle circle;
            circle.center = Physics::ToB2Vec2(collider->offset);
            circle.radius = collider->radius * std::max(trans.scale.x, trans.scale.y);
            b2CreateCircleShape(bodyId, &shapeDef, &circle);
        }
    }
}

void PhysicsSystem::SyncTransformToBody(Entity, RigidBodyComponent& rb, TransformComponent& trans) {
    if (!rb.IsValid()) return;

    // Optimization: Check if transform actually changed
    const float kEpsilon = 0.0001f;
    bool posSame = std::abs(trans.position.x - rb.lastSyncedPosition.x) <= kEpsilon && 
                   std::abs(trans.position.y - rb.lastSyncedPosition.y) <= kEpsilon;
    bool rotSame = std::abs(trans.rotation - rb.lastSyncedRotation) <= kEpsilon;

    if (posSame && rotSame) {
        return;
    }

    b2BodyId bodyId = Physics::ToB2BodyId(rb.bodyHandle);
    b2Body_SetTransform(bodyId, Physics::ToB2Vec2(trans.position), b2MakeRot(trans.rotation * 0.0174533f));
    
    // Update synced state
    rb.lastSyncedPosition = trans.position;
    rb.lastSyncedRotation = trans.rotation;
    
    // If we moved a static/kinematic body, we might need to wake it up or wake up touching bodies
    // b2Body_SetTransform usually handles this, but explicit wake might be needed if we want immediate response
}

void PhysicsSystem::SyncBodyToTransform(Entity, RigidBodyComponent& rb, TransformComponent& trans) {
    if (!rb.IsValid()) return;
    b2BodyId bodyId = Physics::ToB2BodyId(rb.bodyHandle);
    
    // Optimization: Only sync if body is awake
    if (!b2Body_IsAwake(bodyId)) return;

    b2Vec2 pos = b2Body_GetPosition(bodyId);
    b2Rot rot = b2Body_GetRotation(bodyId);
    
    trans.position = Physics::ToVector2(pos);
    trans.rotation = b2Rot_GetAngle(rot) * 57.2958f;
}

void PhysicsSystem::DrawDebug(Registry& reg) {
    // Simple debug draw for colliders
    reg.ForEach<RigidBodyComponent, PhysicsColliderComponent, TransformComponent>(
        [](Entity, RigidBodyComponent&, PhysicsColliderComponent& col, TransformComponent& trans) {
            float rotRad = trans.rotation * 0.0174533f;
            Vector2 center = trans.position + col.offset.Rotate(rotRad);
            if (col.shape == ColliderShape::Box) {
                // Draw Rect
                // This is an approximation, doesn't handle rotation of the box perfectly if the box itself is rotated relative to body
                // But for now, assuming box is axis aligned with body
                
                // We need to draw a rotated rectangle
                Vector2 size = col.size * trans.scale;
                
                // Calculate corners
                Vector2 halfSize = size * 0.5f;
                Vector2 p1 = center + Vector2(-halfSize.x, -halfSize.y).Rotate(rotRad);
                Vector2 p2 = center + Vector2(halfSize.x, -halfSize.y).Rotate(rotRad);
                Vector2 p3 = center + Vector2(halfSize.x, halfSize.y).Rotate(rotRad);
                Vector2 p4 = center + Vector2(-halfSize.x, halfSize.y).Rotate(rotRad);
                
                Renderer::DrawLine(p1, p2, Color::Green());
                Renderer::DrawLine(p2, p3, Color::Green());
                Renderer::DrawLine(p3, p4, Color::Green());
                Renderer::DrawLine(p4, p1, Color::Green());
                
            } else if (col.shape == ColliderShape::Circle) {
                float radius = col.radius * std::max(trans.scale.x, trans.scale.y);
                Renderer::DrawCircle(center, radius, Color::Green());
                // Draw line to show rotation
                Vector2 end = center + Vector2(radius, 0).Rotate(rotRad);
                Renderer::DrawLine(center, end, Color::Green());
            }
        }
    );
}

void PhysicsSystem::OnContact(const Physics::ContactEvent& event) {
    if (!m_CurrentRegistry) return;

    // Get bodies from shapes
    b2ShapeId shapeA = event.shapeA;
    b2ShapeId shapeB = event.shapeB;
    
    if (!b2Shape_IsValid(shapeA) || !b2Shape_IsValid(shapeB)) return;

    b2BodyId bodyA = b2Shape_GetBody(shapeA);
    b2BodyId bodyB = b2Shape_GetBody(shapeB);
    
    // Get entities from user data
    Entity entityA = static_cast<Entity>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyA)));
    Entity entityB = static_cast<Entity>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyB)));

    // Check if either shape is a sensor
    bool isSensorA = b2Shape_IsSensor(shapeA);
    bool isSensorB = b2Shape_IsSensor(shapeB);
    bool isTriggerEvent = isSensorA || isSensorB;
    
    auto UpdateContact = [&](Entity me, Entity other, bool begin) {
        if (m_CurrentRegistry->IsAlive(me)) {
            if (auto* col = m_CurrentRegistry->Get<PhysicsColliderComponent>(me)) {
                if (begin) {
                    // Add if not exists
                    if (std::find(col->contacts.begin(), col->contacts.end(), other) == col->contacts.end()) {
                        col->contacts.push_back(other);
                        if (isTriggerEvent) {
                            if (col->onTriggerEnter) col->onTriggerEnter(other);
                        } else {
                            if (col->onCollisionEnter) col->onCollisionEnter(other);
                        }
                    }
                } else {
                    // Remove
                    auto it = std::remove(col->contacts.begin(), col->contacts.end(), other);
                    if (it != col->contacts.end()) {
                        col->contacts.erase(it, col->contacts.end());
                        if (isTriggerEvent) {
                            if (col->onTriggerExit) col->onTriggerExit(other);
                        } else {
                            if (col->onCollisionExit) col->onCollisionExit(other);
                        }
                    }
                }
                col->colliding = !col->contacts.empty();
            }
        }
    };

    UpdateContact(entityA, entityB, event.isBegin);
    UpdateContact(entityB, entityA, event.isBegin);
}

void AnimationSystem::Tick(Registry& reg, float deltaTime) {
    reg.ForEach<AnimationComponent, SpriteComponent>([deltaTime](Entity, AnimationComponent& anim, SpriteComponent& sprite) {
        if (!anim.playing) {
            return;
        }

        anim.animator.Update(deltaTime);
        if (auto* frame = anim.animator.GetCurrentFrameData()) {
            sprite.sprite.textureRect = frame->uvRect;
            sprite.sprite.transform.origin = frame->pivot;
        }
    });
}

void SpriteRenderSystem::Tick(Registry& reg, float /*deltaTime*/) {
    // Find active camera
    Camera2D camera;
    bool foundCamera = false;
    reg.ForEach<CameraComponent, TransformComponent>([&](Entity, CameraComponent& cam, TransformComponent& trans) {
        if (cam.active && !foundCamera) {
            camera = cam.camera;
            camera.SetPosition(trans.position);
            foundCamera = true;
        }
    });

    if (foundCamera) {
        Renderer::SetCamera(camera);
        Renderer::BeginSpriteBatch(&camera);
    } else {
        // Reset to auto projection if no camera found
        Renderer::ConfigureAutoProjection(true);
        Renderer::BeginSpriteBatch(nullptr);
    }

    struct DrawItem {
        int layer;
        Sprite* sprite;
        TransformComponent* transform;
        Texture* texture;
        bool transparent;
    };

    std::vector<DrawItem> opaque;
    std::vector<DrawItem> transparent;
    opaque.reserve(reg.AliveCount());
    transparent.reserve(reg.AliveCount());

    reg.ForEach<TransformComponent, SpriteComponent>([&](Entity, TransformComponent& transform, SpriteComponent& sprite) {
        auto* tex = sprite.sprite.GetTexture().get();
        if (!sprite.visible || !tex) {
            return;
        }
        DrawItem item{sprite.layer, &sprite.sprite, &transform, tex, sprite.transparent};
        (sprite.transparent ? transparent : opaque).push_back(item);
    });

    auto sorter = [](const DrawItem& a, const DrawItem& b) {
        if (a.layer != b.layer) return a.layer < b.layer;
        // Remove texture sorting to preserve draw order (Painter's Algorithm)
        // If we want depth sorting, we should use Y position, but for now rely on insertion order
        return false; 
    };
    std::stable_sort(opaque.begin(), opaque.end(), sorter);
    std::stable_sort(transparent.begin(), transparent.end(), sorter); // back-to-front по layer

    auto drawList = [&](std::vector<DrawItem>& list) {
        for (auto& item : list) {
            item.sprite->transform.position = item.transform->position;
            item.sprite->transform.scale = item.transform->scale;
            item.sprite->transform.rotation = item.transform->rotation;
            item.sprite->transform.origin = item.transform->origin;
            if (m_DrawCallback) {
                m_DrawCallback(*item.sprite);
            } else {
                // Submit to batch
                Renderer::SubmitSprite(*item.sprite);
            }
        }
    };

    // сначала непрозрачные, затем прозрачные для корректного альфа-блендинга
    drawList(opaque);
    drawList(transparent);
    
    Renderer::FlushSpriteBatch();
}

void TilemapRenderSystem::Tick(Registry& reg, float /*deltaTime*/) {
    Camera2D camera;
    bool foundCamera = false;
    
    // Try to find an active camera component
    reg.ForEach<CameraComponent, TransformComponent>([&](Entity, CameraComponent& cam, TransformComponent& trans) {
        if (cam.active && !foundCamera) {
            camera = cam.camera;
            camera.SetPosition(trans.position);
            foundCamera = true;
        }
    });

    auto* backend = Renderer::GetBackend();
    if (!backend) return;

    if (foundCamera) {
        backend->BeginSpriteBatch(&camera);
    } else {
        backend->BeginSpriteBatch(nullptr);
    }

    reg.ForEach<TilemapComponent>([&](Entity, TilemapComponent& tc) {
        if (tc.visible && tc.tilemap) {
            tc.tilemap->Render(backend, camera);
        }
    });

    backend->FlushSpriteBatch();
}

void MovementSystem::Tick(Registry& reg, float deltaTime) {
    reg.ForEach<TransformComponent, VelocityComponent>([&reg, deltaTime](Entity e, TransformComponent& trans, VelocityComponent& vel) {
        // Skip entities with RigidBodyComponent - let PhysicsSystem handle them
        if (reg.Has<RigidBodyComponent>(e)) return;

        trans.position += vel.velocity * deltaTime;
        trans.rotation += vel.angularVelocity * deltaTime;
    });
}

void PathFollowSystem::Tick(Registry& reg, float deltaTime) {
    reg.ForEach<TransformComponent, PathFollowerComponent>([deltaTime](Entity, TransformComponent& trans, PathFollowerComponent& follower) {
        if (!follower.active || !follower.path) return;

        // Update t
        float deltaT = follower.speed * deltaTime;
        if (follower.reverse) {
            follower.currentT -= deltaT;
        } else {
            follower.currentT += deltaT;
        }

        // Handle looping/pingpong
        if (follower.currentT > 1.0f) {
            if (follower.pingPong) {
                follower.currentT = 1.0f - (follower.currentT - 1.0f);
                follower.reverse = true;
            } else if (follower.loop) {
                follower.currentT -= 1.0f;
            } else {
                follower.currentT = 1.0f;
                follower.active = false;
            }
        } else if (follower.currentT < 0.0f) {
            if (follower.pingPong) {
                follower.currentT = -follower.currentT;
                follower.reverse = false;
            } else if (follower.loop) {
                follower.currentT += 1.0f;
            } else {
                follower.currentT = 0.0f;
                follower.active = false;
            }
        }

        // Update position
        trans.position = follower.path->GetPoint(follower.currentT);
    });
}

void CollisionSystem::Tick(Registry& reg, float /*deltaTime*/) {
    std::vector<Entity> colliders;
    colliders.reserve(reg.AliveCount());
    reg.ForEach<ColliderComponent, TransformComponent>([&](Entity e, ColliderComponent&, TransformComponent&) {
        colliders.push_back(e);
    });

    // Reset colliding flags
    for (auto e : colliders) {
        if (auto* c = reg.Get<ColliderComponent>(e)) {
            c->colliding = false;
        }
    }

    for (size_t i = 0; i < colliders.size(); ++i) {
        for (size_t j = i + 1; j < colliders.size(); ++j) {
            Entity a = colliders[i];
            Entity b = colliders[j];
            auto* ca = reg.Get<ColliderComponent>(a);
            auto* cb = reg.Get<ColliderComponent>(b);
            auto* ta = reg.Get<TransformComponent>(a);
            auto* tb = reg.Get<TransformComponent>(b);
            if (!ca || !cb || !ta || !tb) continue;

            Rect ra = Rect::FromCenter(ta->position + ca->offset, ca->size);
            Rect rb = Rect::FromCenter(tb->position + cb->offset, cb->size);
            if (ra.Intersects(rb)) {
                ca->colliding = true;
                cb->colliding = true;
                // Доп. обработка (триггеры/урон) оставляем игровой логике
            }
        }
    }
}

void GroundCheckSystem::Tick(Registry& reg, float /*deltaTime*/) {
    reg.ForEach<PlayerMovementComponent, TransformComponent, PhysicsColliderComponent>([&](Entity, PlayerMovementComponent& move, TransformComponent& trans, PhysicsColliderComponent& col) {
        // Raycast down to check for ground
        // Start from the bottom of the collider
        float halfHeight = col.size.y * 0.5f;
        Vector2 start = trans.position + col.offset;
        start.y += halfHeight + 0.1f; // Slightly below
        
        Vector2 end = start + Vector2{0.0f, 10.0f}; // Check 10 pixels down

        auto hit = m_PhysicsWorld.RayCast(start, end);
        move.canJump = hit.hit;
    });
}

void PlatformBehaviorSystem::Tick(Registry& reg, float /*deltaTime*/) {
    reg.ForEach<PlatformBehaviorComponent, TransformComponent, RigidBodyComponent, PhysicsColliderComponent>(
        [&](Entity, PlatformBehaviorComponent& pb, TransformComponent& trans, RigidBodyComponent& rb, PhysicsColliderComponent& col) {
            if (!pb.stayOnPlatform || !rb.IsValid()) return;

            Vector2 velocity = m_PhysicsWorld.GetLinearVelocity(rb.bodyHandle);
            if (std::abs(velocity.x) < 0.1f) return; // Not moving horizontally

            float lookAhead = (velocity.x > 0 ? 1.0f : -1.0f) * pb.edgeLookAhead;
            
            // Start raycast from bottom of collider + lookAhead
            float halfHeight = col.size.y * 0.5f;
            Vector2 start = trans.position + col.offset;
            start.x += lookAhead;
            start.y += halfHeight + 0.1f; // Slightly below

            Vector2 end = start + Vector2{0.0f, 20.0f}; // Check down

            auto hit = m_PhysicsWorld.RayCast(start, end);
            if (!hit.hit) {
                // Cliff detected! Reverse velocity
                velocity.x = -velocity.x;
                m_PhysicsWorld.SetLinearVelocity(rb.bodyHandle, velocity);
            }
        }
    );
}

void PlayerInputSystem::Tick(Registry& reg, float deltaTime) {
    reg.ForEach<PlayerTag, VelocityComponent, PlayerMovementComponent>([this, deltaTime, &reg](Entity e, PlayerTag&, VelocityComponent& vel, PlayerMovementComponent& move) {
        float speed = move.moveSpeed > 0.0f ? move.moveSpeed : moveSpeed;
        vel.velocity = {0.0f, 0.0f};
        auto state = m_Provider ? m_Provider() : InputState{
            Input::IsKeyDown(KeyCode::A) || Input::IsKeyDown(KeyCode::Left),
            Input::IsKeyDown(KeyCode::D) || Input::IsKeyDown(KeyCode::Right),
            Input::IsKeyDown(KeyCode::W) || Input::IsKeyDown(KeyCode::Up),
            Input::IsKeyDown(KeyCode::S) || Input::IsKeyDown(KeyCode::Down),
            Input::IsKeyDown(KeyCode::Space)
        };

        if (auto* ic = reg.Get<InputComponent>(e)) {
            state.left = ic->left;
            state.right = ic->right;
            state.up = ic->up;
            state.down = ic->down;
            state.jump = ic->jump;
        }

        if (state.left)  vel.velocity.x -= speed;
        if (state.right) vel.velocity.x += speed;
        if (state.up)    vel.velocity.y -= speed;
        if (state.down)  vel.velocity.y += speed;
    });
}

void CameraFollowSystem::Tick(Registry& reg, float deltaTime) {
    // Find active camera
    CameraComponent* activeCam = nullptr;
    reg.ForEach<CameraComponent>([&](Entity, CameraComponent& cam) {
        if (cam.active) activeCam = &cam;
    });
    
    if (!activeCam) return;

    reg.ForEach<CameraFollowComponent, TransformComponent>([&](Entity, CameraFollowComponent& follow, TransformComponent& t) {
        // Простой линейный интерполяционный фоллоу
        Vector2 target = t.position;
        Vector2 current = activeCam->camera.GetPosition();
        Vector2 newPos = {
            current.x + (target.x - current.x) * follow.smoothness * deltaTime,
            current.y + (target.y - current.y) * follow.smoothness * deltaTime
        };
        activeCam->camera.SetPosition(newPos);
    });
}

void AudioSystem::Tick(Registry& reg, float /*deltaTime*/) {
    // 1. Update Listener Position (Camera)
    bool listenerSet = false;
    reg.ForEach<CameraComponent, TransformComponent>([&](Entity, CameraComponent& cam, TransformComponent& trans) {
        if (cam.active && !listenerSet) {
            Audio::SetListenerPosition(trans.position);
            listenerSet = true;
        }
    });

    // 2. Update Audio Components
    reg.ForEach<AudioComponent>([&](Entity e, AudioComponent& audio) {
        // Load sound if needed
        if (!audio.sound && !audio.path.empty()) {
            SoundType type = SoundType::Static;
            if (audio.path.find(".ogg") != std::string::npos || audio.path.find(".mp3") != std::string::npos) {
                type = SoundType::Stream;
            }
            audio.sound = Sound::Create(audio.path, type);
        }

        if (!audio.sound) return;

        // Update properties
        audio.sound->SetLooping(audio.loop);
        audio.sound->SetVolume(audio.volume);
        audio.sound->SetSpatial(audio.spatial);
        
        if (audio.spatial) {
            audio.sound->SetMinDistance(audio.minDistance);
            audio.sound->SetMaxDistance(audio.maxDistance);
            
            // Update position if entity has transform
            if (auto* trans = reg.Get<TransformComponent>(e)) {
                audio.sound->SetPosition(trans->position);
            }
        }

        // Handle Play Request
        if (audio.playRequested && !audio.sound->IsPlaying()) {
            audio.sound->Play();
            audio.playRequested = false;
        }
    });
}

void StatsSystem::Tick(Registry& reg, float deltaTime) {
    reg.ForEach<StatsComponent>([this, deltaTime](Entity, StatsComponent& stats) {
        // Регенерация
        if (regenHealthPerSec > 0.0f) {
            stats.health = std::min(stats.maxHealth, stats.health + static_cast<int>(regenHealthPerSec * deltaTime));
        }
        if (regenEnergyPerSec > 0.0f) {
            stats.energy = std::min(stats.maxEnergy, stats.energy + static_cast<int>(regenEnergyPerSec * deltaTime));
        }

        // Нормализация
        stats.health = std::clamp(stats.health, 0, stats.maxHealth);
        stats.energy = std::clamp(stats.energy, 0, stats.maxEnergy);
    });
}

void InputStateSystem::Tick(Registry& reg, float /*deltaTime*/) {
    reg.ForEach<InputComponent>([](Entity, InputComponent& ic) {
        ic.left = Input::IsKeyDown(KeyCode::A) || Input::IsKeyDown(KeyCode::Left);
        ic.right = Input::IsKeyDown(KeyCode::D) || Input::IsKeyDown(KeyCode::Right);
        ic.up = Input::IsKeyDown(KeyCode::W) || Input::IsKeyDown(KeyCode::Up);
        ic.down = Input::IsKeyDown(KeyCode::S) || Input::IsKeyDown(KeyCode::Down);
        ic.jump = Input::IsKeyDown(KeyCode::Space);
        ic.attack = Input::IsMouseButtonDown(MouseButton::Left);
    });
}

void ParticleSystemSystem::Tick(Registry& reg, float deltaTime) {
    reg.ForEach<ParticleEmitterComponent, TransformComponent>([deltaTime](Entity, ParticleEmitterComponent& emitter, TransformComponent& t) {
        if (!emitter.system) {
            return;
        }

        // Обновляем конфиг и эмиссию
        emitter.emissionTimer += deltaTime;
        if (emitter.playing && emitter.emissionRate > 0.0f) {
            float interval = 1.0f / emitter.emissionRate;
            while (emitter.emissionTimer >= interval) {
                emitter.system->Emit(1);
                emitter.emissionTimer -= interval;
            }
        }

        emitter.system->Update(deltaTime);

        // Рендерим частицы как квадраты
        const auto& particles = emitter.system->GetParticles();
        for (const auto& p : particles) {
            if (!p.active) continue;
            Color c = p.color;
            c.a *= std::max(0.0f, 1.0f - (p.age / p.lifetime) * p.fadeOut);
            Renderer::DrawParticle(t.position + p.position, p.size, c, p.rotation);
        }
    });
}

void DeathSystem::Tick(Registry& reg, float /*deltaTime*/) {
    std::vector<Entity> toDestroy;
    reg.ForEach<HealthComponent>([&](Entity e, HealthComponent& h) {
        if (h.IsDead()) {
            toDestroy.push_back(e);
        }
    });
    for (auto e : toDestroy) {
        reg.DestroyEntity(e);
    }
}

void HudRenderSystem::Tick(Registry&, float) {
    // Placeholder for HUD rendering
}

void DamageSystem::Tick(Registry&, float) {
    // Placeholder for damage processing
}

void CameraSystem::Tick(Registry&, float) {
    // Placeholder for camera updates
}

void NativeScriptSystem::Tick(Registry& reg, float deltaTime) {
    reg.ForEach<NativeScriptComponent>([this, deltaTime](Entity e, NativeScriptComponent& nsc) {
        if (!nsc.Instance) {
            nsc.Instance.reset(nsc.InstantiateScript());
            nsc.Instance->m_GameObject = GameObject(e, m_Scene);
            nsc.Instance->OnCreate();
        }
        nsc.Instance->OnUpdate(deltaTime);
    });
}

void RaycastSystem::Tick(Registry&, float) {
    // Nothing to do in Tick for now, this is mostly a utility system
}

Entity RaycastSystem::RaycastFromScreen(Registry& reg, const Vector2& screenPos, const Camera2D& camera) {
    Vector2 worldPos = camera.ScreenToWorld(screenPos);
    
    // We want to find what is under the cursor.
    // Since Box2D raycasts are lines, a point check is better done via AABB query or small box query.
    // However, PhysicsWorld::RayCast is a line segment.
    // Let's use a tiny box query or just check all colliders manually if we want "pixel perfect" clicking on sprites without physics.
    // But the user asked for "Raycast support" and "fix clicking".
    
    // Approach 1: Physics Query (Best for physics objects)
    // We'll create a tiny AABB around the mouse point and query the world.
    // Since PhysicsWorld doesn't expose AABB query directly yet, let's implement a simple point check using existing RayCast
    // by casting a ray from "camera near plane" to "camera far plane" conceptually, but this is 2D.
    // In 2D, "under the mouse" means the point (worldPos) is inside a shape.
    
    // Let's iterate all physics bodies and check if the point is inside.
    // This is slow for many bodies, but accurate.
    // A better way is to add QueryAABB to PhysicsWorld.
    
    // For now, let's use a manual check against ECS components for flexibility (supports non-physics sprites too if we wanted).
    // But the user specifically mentioned "Raycast".
    
    // Let's add QueryPoint to PhysicsWorld!
    // But I cannot easily modify PhysicsWorld header without reading it again to be sure.
    // I'll stick to a manual iteration over PhysicsColliderComponents for now, as it's safe and easy.
    
    Entity hitEntity = kInvalidEntity;
    int maxLayer = -1; // To handle overlapping sprites/bodies, pick the one on top (highest layer)

    reg.ForEach<TransformComponent, PhysicsColliderComponent>([&](Entity e, TransformComponent& trans, PhysicsColliderComponent& col) {
        // Calculate collider world position/shape
        // Assuming Box for now
        if (col.shape == ColliderShape::Box) {
            // Apply rotation
            float rotRad = trans.rotation * 0.0174533f;
            Vector2 center = trans.position + col.offset.Rotate(rotRad);
            Vector2 size = col.size * trans.scale;
            
            // To check point in rotated rectangle:
            // Rotate point back by -rot around center
            Vector2 localPoint = (worldPos - center).Rotate(-rotRad);
            
            if (std::abs(localPoint.x) <= size.x * 0.5f && std::abs(localPoint.y) <= size.y * 0.5f) {
                // Hit!
                // Check layer if sprite exists
                int layer = 0;
                if (auto* sprite = reg.Get<SpriteComponent>(e)) {
                    layer = sprite->layer;
                }
                
                if (layer > maxLayer) {
                    maxLayer = layer;
                    hitEntity = e;
                }
            }
        } else if (col.shape == ColliderShape::Circle) {
            float rotRad = trans.rotation * 0.0174533f;
            Vector2 center = trans.position + col.offset.Rotate(rotRad);
            float radius = col.radius * std::max(trans.scale.x, trans.scale.y);
            
            if ((worldPos - center).LengthSquared() <= radius * radius) {
                 // Hit!
                int layer = 0;
                if (auto* sprite = reg.Get<SpriteComponent>(e)) {
                    layer = sprite->layer;
                }
                
                if (layer > maxLayer) {
                    maxLayer = layer;
                    hitEntity = e;
                }
            }
        }
    });
    
    return hitEntity;
}

// New Raycast method using PhysicsWorld
Physics::PhysicsWorld::RayCastHit RaycastSystem::Raycast(const Vector2& start, const Vector2& end) {
    return m_PhysicsWorld.RayCast(start, end);
}

} // namespace SAGE::ECS