#include "Box2DBackend.h"
#include "Core/Logger.h"
#include "ECS/Registry.h"
#include "ECS/Components/Core/TransformComponent.h"
#include "ECS/Components/Physics/PhysicsComponent.h"
#include "ECS/Components/Physics/ColliderComponent.h"
#include "Graphics/API/Renderer.h"  // For debug draw

#include <box2d/box2d.h>
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace SAGE::Physics {

// Unit conversion
// Box2D uses Y-up coordinate system (mathematical)
// SAGE Engine world uses the same handedness: Y increases UP, ground is negative
// Therefore we keep Y as-is while converting between systems
static constexpr float PIXELS_PER_METER = 100.0f;
static inline float ToMeters(float pixels) { return pixels / PIXELS_PER_METER; }
static inline float ToPixels(float meters) { return meters * PIXELS_PER_METER; }
static inline b2Vec2 ToB2Vec2(const Vector2& vec) { 
    return b2Vec2{ToMeters(vec.x), ToMeters(vec.y)};
}
static inline Vector2 FromB2Vec2(const b2Vec2& vec) { 
    return Vector2(ToPixels(vec.x), ToPixels(vec.y));
}

Box2DBackend::Box2DBackend() { Logger::Trace("Box2DBackend: Constructor"); }
Box2DBackend::~Box2DBackend() { Clear(); }

void Box2DBackend::Initialize(const PhysicsSettings& settings) {
    Logger::Info("Box2DBackend: Initializing Box2D v3.x");
    
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = ToB2Vec2(settings.gravity);
    m_worldId = b2CreateWorld(&worldDef);
    
    if (!b2World_IsValid(m_worldId)) {
        Logger::Error("Box2DBackend: Failed to create world!");
        return;
    }
    
    Logger::Info("Box2DBackend: World created with gravity ({}, {})", settings.gravity.x, settings.gravity.y);
    m_settings = settings;
    
    // Enable Continuous Collision Detection at world level if requested
    if (settings.enableCCD) {
        b2World_EnableContinuous(m_worldId, true);
        Logger::Info("Box2DBackend: CCD enabled");
    }
    
    // Setup contact callback
    b2World_SetPreSolveCallback(m_worldId, PreSolveCallback, this);
}

bool Box2DBackend::PreSolveCallback(b2ShapeId shapeIdA, b2ShapeId shapeIdB, 
                                    b2Vec2 point, b2Vec2 normal, void* context) {
    auto* backend = static_cast<Box2DBackend*>(context);
    
    b2BodyId bodyIdA = b2Shape_GetBody(shapeIdA);
    b2BodyId bodyIdB = b2Shape_GetBody(shapeIdB);
    
    auto itA = backend->m_bodyIndexToEntity.find(bodyIdA.index1);
    auto itB = backend->m_bodyIndexToEntity.find(bodyIdB.index1);
    
    if (itA != backend->m_bodyIndexToEntity.end() && 
        itB != backend->m_bodyIndexToEntity.end()) {
        
        Contact contact;
        contact.entityA = itA->second;
        contact.entityB = itB->second;
        contact.normal = FromB2Vec2(normal);
        contact.contactPoints.push_back(FromB2Vec2(point));
        
        // Note: Box2D v3 callbacks are called during physics step (single-threaded)
        // so push_back is safe here. If multithreading is added later, use mutex.
        backend->m_contacts.push_back(contact);
        
        if (backend->m_contactCallback) {
            backend->m_contactCallback(contact);
        }
    }
    
    return true; // true = resolve collision, false = ignore
}

void Box2DBackend::Step(ECS::Registry& registry, float deltaTime) {
    if (!b2World_IsValid(m_worldId)) return;
    
    // Clear previous frame contacts
    m_contacts.clear();
    
    // In Box2D v3, subStepCount is the number of sub-steps for stability
    // NOT the same as velocity/position iterations
    // Recommended: 4 substeps for 60Hz, can go higher for better accuracy
    int subStepCount = 4;  // Fixed substeps for stability
    
    // Step the world
    b2World_Step(m_worldId, deltaTime, subStepCount);
}

void Box2DBackend::SyncTransforms(ECS::Registry& registry) {
    if (!b2World_IsValid(m_worldId)) return;
    
    // Sync physics bodies back to ECS transforms and rigidbodies
    for (const auto& [entity, bodyId] : m_entityToBody) {
        if (!b2Body_IsValid(bodyId)) continue;
        
        auto* transform = registry.GetComponent<ECS::TransformComponent>(entity);
        auto* physics = registry.GetComponent<ECS::PhysicsComponent>(entity);
        
        if (transform) {
            b2Vec2 pos = b2Body_GetPosition(bodyId);
            b2Rot rot = b2Body_GetRotation(bodyId);
            transform->position = FromB2Vec2(pos);
            transform->SetRotation(b2Rot_GetAngle(rot) * (180.0f / static_cast<float>(M_PI)));
        }
        
        // Sync velocity and state back to ECS component
        if (physics && physics->type == ECS::PhysicsBodyType::Dynamic) {
            b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
            physics->velocity = FromB2Vec2(vel);
            physics->angularVelocity = b2Body_GetAngularVelocity(bodyId);
            
            // Sync mass data (can change in Box2D if shapes are added/removed)
            b2MassData massData = b2Body_GetMassData(bodyId);
            if (massData.mass > 0.0f) {
                physics->mass = massData.mass;
                physics->inertia = massData.rotationalInertia;
            }
            
            // Sync sleep state for rendering optimization
            physics->isSleeping = !b2Body_IsAwake(bodyId);
        }
    }
}

bool Box2DBackend::CreateBody(ECS::Entity entity, ECS::Registry& registry) {
    if (!b2World_IsValid(m_worldId)) return false;
    if (m_entityToBody.find(entity) != m_entityToBody.end()) return true;
    
    auto* transform = registry.GetComponent<ECS::TransformComponent>(entity);
    auto* physics = registry.GetComponent<ECS::PhysicsComponent>(entity);
    
    if (!transform || !physics) {
        Logger::Warning("Box2DBackend: Missing components for entity {}", static_cast<uint32_t>(entity));
        return false;
    }
    
    // Create body definition
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = ToB2Vec2(transform->position);
    bodyDef.rotation = b2MakeRot(transform->GetRotation() * (static_cast<float>(M_PI) / 180.0f));
    
    if (physics->type == ECS::PhysicsBodyType::Static) bodyDef.type = b2_staticBody;
    else if (physics->type == ECS::PhysicsBodyType::Kinematic) bodyDef.type = b2_kinematicBody;
    else bodyDef.type = b2_dynamicBody;
    
    bodyDef.linearVelocity = ToB2Vec2(physics->velocity);
    bodyDef.angularVelocity = physics->angularVelocity;
    bodyDef.gravityScale = physics->gravityScale;
    bodyDef.linearDamping = physics->linearDamping;
    bodyDef.angularDamping = physics->angularDamping;
    bodyDef.enableSleep = true;
    
    b2BodyId bodyId = b2CreateBody(m_worldId, &bodyDef);
    if (!b2Body_IsValid(bodyId)) {
        Logger::Error("Box2DBackend: Failed to create body");
        return false;
    }
    
    m_entityToBody[entity] = bodyId;
    m_bodyIndexToEntity[bodyId.index1] = entity;
    
    bool shapeCreated = false;
    
    // ✅ НОВОЕ: Поддержка универсального ColliderComponent
    if (registry.HasComponent<ECS::ColliderComponent>(entity)) {
        auto* collider = registry.GetComponent<ECS::ColliderComponent>(entity);
        if (collider) {
            // Setup shape definition with material properties from collider
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.density = collider->GetDensity();
            shapeDef.material.friction = collider->GetFriction();
            shapeDef.material.restitution = collider->GetRestitution();
            shapeDef.isSensor = collider->IsTrigger();
            shapeDef.enableContactEvents = true;
            shapeDef.enableHitEvents = false;
            
            b2Vec2 offset = ToB2Vec2(collider->GetOffset());
            
            switch (collider->GetType()) {
                case ECS::ColliderComponent::Type::Circle: {
                    b2Circle circle;
                    circle.center = offset;
                    circle.radius = ToMeters(collider->GetCircleRadius());
                    
                    b2ShapeId shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
                    shapeCreated = !B2_IS_NULL(shapeId);
                    break;
                }
                
                case ECS::ColliderComponent::Type::Box: {
                    Vector2 size = collider->GetBoxSize();
                    b2Polygon box;
                    
                    if (offset.x != 0.0f || offset.y != 0.0f) {
                        box = b2MakeOffsetBox(
                            ToMeters(size.x * 0.5f),
                            ToMeters(size.y * 0.5f),
                            offset,
                            b2Rot_identity
                        );
                    } else {
                        box = b2MakeBox(
                            ToMeters(size.x * 0.5f),
                            ToMeters(size.y * 0.5f)
                        );
                    }
                    
                    b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);
                    shapeCreated = !B2_IS_NULL(shapeId);
                    break;
                }
                
                case ECS::ColliderComponent::Type::Capsule: {
                    // Box2D doesn't have native capsule, approximate with circle + box
                    float radius = collider->GetCapsuleRadius();
                    float height = collider->GetCapsuleHeight();
                    Vector2 axis = collider->GetCapsuleAxis();
                    
                    // Create capsule as circle at top, circle at bottom, box in middle
                    // For simplicity, create 2 circles + 1 box
                    float halfHeight = height * 0.5f;
                    
                    // Top circle
                    b2Circle topCircle;
                    topCircle.center = b2Vec2{
                        offset.x + ToMeters(axis.x * halfHeight),
                        offset.y + ToMeters(axis.y * halfHeight)
                    };
                    topCircle.radius = ToMeters(radius);
                    b2CreateCircleShape(bodyId, &shapeDef, &topCircle);
                    
                    // Bottom circle
                    b2Circle bottomCircle;
                    bottomCircle.center = b2Vec2{
                        offset.x - ToMeters(axis.x * halfHeight),
                        offset.y - ToMeters(axis.y * halfHeight)
                    };
                    bottomCircle.radius = ToMeters(radius);
                    b2CreateCircleShape(bodyId, &shapeDef, &bottomCircle);
                    
                    // Middle box
                    float angle = std::atan2(axis.y, axis.x);
                    b2Polygon middleBox = b2MakeOffsetBox(
                        ToMeters(radius),
                        ToMeters(height * 0.5f),
                        offset,
                        b2MakeRot(angle)
                    );
                    b2CreatePolygonShape(bodyId, &shapeDef, &middleBox);
                    
                    shapeCreated = true;
                    break;
                }
                
                case ECS::ColliderComponent::Type::Polygon: {
                    const auto& vertices = collider->GetPolygonVertices();
                    constexpr int maxVerts = 8; // Box2D v3 max polygon vertices
                    
                    if (vertices.size() >= 3 && vertices.size() <= static_cast<size_t>(maxVerts)) {
                        b2Vec2 b2Vertices[8]; // Static array with fixed size
                        for (size_t i = 0; i < vertices.size(); ++i) {
                            b2Vertices[i] = ToB2Vec2(vertices[i]);
                        }
                        
                        b2Hull hull = b2ComputeHull(b2Vertices, static_cast<int>(vertices.size()));
                        b2Polygon polygon = b2MakePolygon(&hull, 0.0f);
                        
                        b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
                        shapeCreated = !B2_IS_NULL(shapeId);
                    } else {
                        Logger::Warning("Box2DBackend: Polygon must have 3-{} vertices, got {}",
                                       maxVerts, vertices.size());
                    }
                    break;
                }
                
                case ECS::ColliderComponent::Type::Compound: {
                    // Create multiple shapes for compound collider
                    const auto& subColliders = collider->GetSubColliders();
                    for (const auto& sub : subColliders) {
                        b2ShapeDef subShapeDef = shapeDef;
                        subShapeDef.density = sub.density;
                        subShapeDef.material.friction = sub.isTrigger ? 0.0f : shapeDef.material.friction;
                        subShapeDef.isSensor = sub.isTrigger;
                        
                        b2Vec2 subOffset = ToB2Vec2(sub.offset);
                        b2Rot subRot = b2MakeRot(sub.rotation * (static_cast<float>(M_PI) / 180.0f));
                        
                        if (sub.type == ECS::ColliderComponent::Type::Circle) {
                            b2Circle circle;
                            circle.center = subOffset;
                            circle.radius = ToMeters(sub.radius);
                            b2CreateCircleShape(bodyId, &subShapeDef, &circle);
                            shapeCreated = true;
                        }
                        else if (sub.type == ECS::ColliderComponent::Type::Box) {
                            b2Polygon box = b2MakeOffsetBox(
                                ToMeters(sub.size.x * 0.5f),
                                ToMeters(sub.size.y * 0.5f),
                                subOffset,
                                subRot
                            );
                            b2CreatePolygonShape(bodyId, &subShapeDef, &box);
                            shapeCreated = true;
                        }
                    }
                    break;
                }
            }
            
            if (!shapeCreated) {
                Logger::Error("Box2DBackend: Failed to create shape for ColliderComponent");
            }
        }
    }
    // Set mass for dynamic bodies (after shape creation)
    if (shapeCreated && physics->type == ECS::PhysicsBodyType::Dynamic) {
        if (physics->mass > 0.0f) {
            b2MassData massData = b2Body_GetMassData(bodyId);
            massData.mass = physics->mass;
            
            if (physics->inertia > 0.0f) {
                massData.rotationalInertia = physics->inertia;
            }
            
            if (physics->fixedRotation) {
                massData.rotationalInertia = 0.0f;
            }
            
            b2Body_SetMassData(bodyId, massData);
        }
        else if (physics->fixedRotation) {
            b2MassData massData = b2Body_GetMassData(bodyId);
            massData.rotationalInertia = 0.0f;
            b2Body_SetMassData(bodyId, massData);
        }
    }
    
    if (!shapeCreated) {
        Logger::Warning("Box2DBackend: No collider component found for entity {}", static_cast<uint32_t>(entity));
    }
    
    return true;
}

void Box2DBackend::DestroyBody(ECS::Entity entity) {
    auto it = m_entityToBody.find(entity);
    if (it != m_entityToBody.end()) {
        if (b2Body_IsValid(it->second)) b2DestroyBody(it->second);
        m_bodyIndexToEntity.erase(it->second.index1);
        m_entityToBody.erase(it);
    }
}

bool Box2DBackend::Raycast(const Vector2& origin, const Vector2& direction, 
                          float maxDistance, RaycastHit& hit) {
    if (!b2World_IsValid(m_worldId)) return false;
    
    Vector2 end = origin + direction * maxDistance;
    
    b2RayResult result = b2World_CastRayClosest(m_worldId, ToB2Vec2(origin), ToB2Vec2(end), b2DefaultQueryFilter());
    
    if (!result.hit) return false;
    
    // Get body from shape
    b2BodyId bodyId = b2Shape_GetBody(result.shapeId);
    
    auto it = m_bodyIndexToEntity.find(bodyId.index1);
    if (it == m_bodyIndexToEntity.end()) return false;
    
    hit.entity = it->second;
    hit.point = FromB2Vec2(result.point);
    hit.normal = FromB2Vec2(result.normal);
    hit.fraction = result.fraction;
    
    return true;
}

void Box2DBackend::QueryAABB(const Vector2& min, const Vector2& max, 
                            std::vector<ECS::Entity>& entities) {
    entities.clear();
    
    if (!b2World_IsValid(m_worldId)) return;
    
    b2AABB aabb;
    aabb.lowerBound = ToB2Vec2(min);
    aabb.upperBound = ToB2Vec2(max);
    
    QueryContext context;
    context.entities = &entities;
    context.bodyMap = &m_bodyIndexToEntity;
    
    b2World_OverlapAABB(m_worldId, aabb, b2DefaultQueryFilter(), QueryAABBCallback, &context);
}

bool Box2DBackend::QueryAABBCallback(b2ShapeId shapeId, void* context) {
    auto* queryContext = static_cast<QueryContext*>(context);
    
    b2BodyId bodyId = b2Shape_GetBody(shapeId);
    auto it = queryContext->bodyMap->find(bodyId.index1);
    
    if (it != queryContext->bodyMap->end()) {
        queryContext->entities->push_back(it->second);
    }
    
    return true; // Continue query
}

void Box2DBackend::SetGravity(const Vector2& gravity) {
    if (b2World_IsValid(m_worldId)) {
        b2World_SetGravity(m_worldId, ToB2Vec2(gravity));
        m_settings.gravity = gravity;
    }
}

Vector2 Box2DBackend::GetGravity() const {
    return m_settings.gravity;
}

void Box2DBackend::SetContactCallback(ContactCallback callback) {
    m_contactCallback = callback;
    // Callback is set in Initialize() with b2World_SetPreSolveCallback
}

void Box2DBackend::SetDebugDraw(bool enabled) {
    m_debugDraw = enabled;
    
    if (!b2World_IsValid(m_worldId)) return;
    
    if (enabled) {
        // Setup Box2D debug draw callbacks
        m_b2DebugDraw = b2DefaultDebugDraw();
        
        m_b2DebugDraw.DrawPolygonFcn = DrawPolygon;
        m_b2DebugDraw.DrawSolidPolygonFcn = DrawSolidPolygon;
        m_b2DebugDraw.DrawCircleFcn = DrawCircle;
        m_b2DebugDraw.DrawSolidCircleFcn = DrawSolidCircle;
        m_b2DebugDraw.DrawLineFcn = DrawSegment;
        m_b2DebugDraw.DrawTransformFcn = DrawTransform;
        m_b2DebugDraw.DrawPointFcn = DrawPoint;
        m_b2DebugDraw.DrawStringFcn = DrawString;
        
        m_b2DebugDraw.context = this;
        
        // Enable drawing options - only shapes for clean visualization
        m_b2DebugDraw.drawShapes = true;           // ✅ Collision shapes
        m_b2DebugDraw.drawJoints = false;          // ❌ No joints yet
        m_b2DebugDraw.drawBounds = false;          // ❌ AABB clutter
        m_b2DebugDraw.drawMass = false;            // ❌ Mass center clutter
        m_b2DebugDraw.drawContactPoints = false;   // ❌ Too much clutter
        m_b2DebugDraw.drawContactNormals = false;  // ❌ Too much clutter
        m_b2DebugDraw.drawContactForces = false;   // ❌ Disabled
        m_b2DebugDraw.drawFrictionForces = false;  // ❌ Disabled
        
        Logger::Info("Box2DBackend: Debug draw enabled (shapes only)");
    } else {
        Logger::Info("Box2DBackend: Debug draw disabled");
    }
}

// Helper: Convert b2HexColor to SAGE Color
static Color FromB2Color(b2HexColor hexColor) {
    // b2HexColor is 0xRRGGBBAA format
    uint8_t r = (hexColor >> 24) & 0xFF;
    uint8_t g = (hexColor >> 16) & 0xFF;
    uint8_t b = (hexColor >> 8) & 0xFF;
    uint8_t a = hexColor & 0xFF;
    return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

// Debug draw callback implementations
void Box2DBackend::DrawPolygon(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* /*context*/) {
    if (vertexCount < 2) return;
    
    Color lineColor = FromB2Color(color);
    
    // Draw wireframe polygon as connected lines
    for (int i = 0; i < vertexCount; ++i) {
        Vector2 p1 = FromB2Vec2(vertices[i]);
        Vector2 p2 = FromB2Vec2(vertices[(i + 1) % vertexCount]);
        Renderer::DrawLine(p1, p2, lineColor, 2.0f);
    }
}

void Box2DBackend::DrawSolidPolygon(b2Transform transform, const b2Vec2* vertices, int vertexCount,
                                     float radius, b2HexColor color, void* /*context*/) {
    if (vertexCount < 3) return;
    
    Color fillColor = FromB2Color(color);
    fillColor.a *= 0.3f; // More transparent for cleaner look
    
    // Transform vertices to world space
    std::vector<Vector2> worldVertices;
    worldVertices.reserve(vertexCount);
    
    // Box2D gives us transform (position + rotation)
    // Vertices are in LOCAL space relative to body center
    float cosAngle = transform.q.c;
    float sinAngle = transform.q.s;
    b2Vec2 worldPos = transform.p;
    
    for (int i = 0; i < vertexCount; ++i) {
        // Rotate vertex by body rotation
        b2Vec2 localVertex = vertices[i];
        b2Vec2 rotatedVertex;
        rotatedVertex.x = cosAngle * localVertex.x - sinAngle * localVertex.y;
        rotatedVertex.y = sinAngle * localVertex.x + cosAngle * localVertex.y;
        
        // Translate to world position and convert to pixels
        b2Vec2 worldVertex;
        worldVertex.x = worldPos.x + rotatedVertex.x;
        worldVertex.y = worldPos.y + rotatedVertex.y;
        
        worldVertices.push_back(FromB2Vec2(worldVertex));
    }
    
    // Draw filled polygon as triangles (fan triangulation)
    for (int i = 1; i < vertexCount - 1; ++i) {
        Renderer::DrawTriangleFilled(worldVertices[0], worldVertices[i], worldVertices[i + 1], fillColor);
    }
    
    // Draw outline
    Color outlineColor = FromB2Color(color);
    outlineColor.a = 0.8f; // Semi-transparent outline
    for (int i = 0; i < vertexCount; ++i) {
        Renderer::DrawLine(worldVertices[i], worldVertices[(i + 1) % vertexCount], outlineColor, 1.5f);
    }
}

void Box2DBackend::DrawCircle(b2Vec2 center, float radius, b2HexColor color, void* /*context*/) {
    Vector2 worldCenter = FromB2Vec2(center);
    float worldRadius = ToPixels(radius);
    Color circleColor = FromB2Color(color);
    circleColor.a = 0.8f; // Semi-transparent
    
    Renderer::DrawCircle(worldCenter, worldRadius, circleColor, 1.5f);
}

void Box2DBackend::DrawSolidCircle(b2Transform transform, float radius, b2HexColor color, void* /*context*/) {
    Vector2 center = FromB2Vec2(transform.p);
    float worldRadius = ToPixels(radius);
    Color fillColor = FromB2Color(color);
    fillColor.a *= 0.3f; // More transparent for cleaner look
    
    // Draw filled circle
    Renderer::DrawCircleFilled(center, worldRadius, fillColor);
    
    // Draw outline
    Color outlineColor = FromB2Color(color);
    outlineColor.a = 0.8f; // Semi-transparent outline
    Renderer::DrawCircle(center, worldRadius, outlineColor, 1.5f);
    
    // Draw radius line to show rotation (thinner and more subtle)
    float angle = b2Rot_GetAngle(transform.q);
    Vector2 radiusEnd = center + Vector2(std::cos(angle) * worldRadius, std::sin(angle) * worldRadius);
    outlineColor.a = 0.6f; // Even more transparent for radius line
    Renderer::DrawLine(center, radiusEnd, outlineColor, 1.0f);
}

void Box2DBackend::DrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* /*context*/) {
    Vector2 start = FromB2Vec2(p1);
    Vector2 end = FromB2Vec2(p2);
    Color lineColor = FromB2Color(color);
    
    Renderer::DrawLine(start, end, lineColor, 2.0f);
}

void Box2DBackend::DrawTransform(b2Transform transform, void* /*context*/) {
    Vector2 position = FromB2Vec2(transform.p);
    float angle = b2Rot_GetAngle(transform.q);
    
    // Draw coordinate axes (X = red, Y = green)
    constexpr float axisLength = 50.0f; // pixels
    
    // X-axis (red)
    Vector2 xAxis = position + Vector2(std::cos(angle) * axisLength, std::sin(angle) * axisLength);
    Renderer::DrawLine(position, xAxis, Color(1.0f, 0.0f, 0.0f, 1.0f), 3.0f);
    
    // Y-axis (green)
    float yAngle = angle + static_cast<float>(M_PI) * 0.5f;
    Vector2 yAxis = position + Vector2(std::cos(yAngle) * axisLength, std::sin(yAngle) * axisLength);
    Renderer::DrawLine(position, yAxis, Color(0.0f, 1.0f, 0.0f, 1.0f), 3.0f);
}

void Box2DBackend::DrawPoint(b2Vec2 p, float size, b2HexColor color, void* /*context*/) {
    Vector2 position = FromB2Vec2(p);
    Color pointColor = FromB2Color(color);
    float worldSize = ToPixels(size);
    
    // Draw point as small filled circle
    Renderer::DrawCircleFilled(position, worldSize, pointColor);
}

void Box2DBackend::DrawString(b2Vec2 p, const char* s, b2HexColor color, void* context) {
    // Text rendering not implemented yet - would require text rendering API
    // For now, just skip it silently
    (void)p;
    (void)s;
    (void)color;
    (void)context;
}

const std::vector<Contact>& Box2DBackend::GetContacts() const {
    return m_contacts;
}

void Box2DBackend::Clear() {
    if (b2World_IsValid(m_worldId)) {
        b2DestroyWorld(m_worldId);
        m_worldId = b2_nullWorldId;  // Reset to null
    }
    m_entityToBody.clear();
    m_bodyIndexToEntity.clear();
    m_contacts.clear();
}

void Box2DBackend::DrawDebug() {
    if (!m_debugDraw || !b2World_IsValid(m_worldId)) return;
    
    // Call Box2D debug draw - will trigger all the callbacks
    b2World_Draw(m_worldId, &m_b2DebugDraw);
}

} // namespace SAGE::Physics
