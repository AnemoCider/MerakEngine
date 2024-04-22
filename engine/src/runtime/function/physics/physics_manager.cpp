#include "runtime/function/physics/physics_manager.h"
#include "runtime/core/math/math.h"
#include "runtime/function/global/global_context.h"

namespace Merak {

//b2World& PhysicsManager::getB2World() {
//    static b2Vec2 _gravity(0.0f, 9.8f);
//    static b2World _world {_gravity};
//    return _world;
//}

void PhysicsManager::tick() {
    // getB2World().Step(1.0f / 60.0f, 8, 3);
}

RigidBody::~RigidBody() {
    // if (body) PhysicsManager::getB2World().DestroyBody(body);
}

void RigidBody::init() {
    /*assert (!body);
    b2BodyDef bodyDef;
    if (body_type == "dynamic")
        bodyDef.type = b2_dynamicBody;
    else if (body_type == "kinematic")
        bodyDef.type = b2_kinematicBody;
    else if (body_type == "static")
        bodyDef.type = b2_staticBody;
    
    bodyDef.gravityScale = gravity_scale;
    bodyDef.position = b2Vec2(x, y);
    bodyDef.bullet = precise;
    bodyDef.angularDamping = angular_friction;
    bodyDef.angle = deg2Rad(rotation);

    body = PhysicsManager::getB2World().CreateBody(&bodyDef);

    if (!has_collider && !has_trigger) {
        b2FixtureDef fixture;
        b2PolygonShape shape;
        fixture.userData.pointer = reinterpret_cast<uintptr_t>(nullptr);
        shape.SetAsBox(width * 0.5f, height * 0.5f);

        fixture.shape = &shape;
        fixture.density = density;
        fixture.isSensor = true;
        body->CreateFixture(&fixture);
    } else {
        if (has_collider) {
            b2FixtureDef fixture;
            fixture.userData.pointer = reinterpret_cast<uintptr_t>(actor);
            fixture.density = density;
            fixture.restitution = bounciness;
            fixture.friction = friction;
            fixture.filter.categoryBits = 0x01;
            b2Shape* shape = nullptr;
            if (collider_type == "box") {
                auto* polyShape = new b2PolygonShape();
                polyShape->SetAsBox(width * 0.5f, height * 0.5f);
                shape = polyShape;
            } else if (collider_type == "circle") {
                auto* circleShape = new b2CircleShape();
                circleShape->m_radius = radius;
                shape = circleShape;
            }
            assert(shape);
            fixture.shape = shape;
            body->CreateFixture(&fixture);
            delete shape;
        }
        if (has_trigger) {
            b2FixtureDef fixture;
            fixture.userData.pointer = reinterpret_cast<uintptr_t>(actor);
            fixture.density = density;
            fixture.restitution = bounciness;
            fixture.friction = friction;
            fixture.filter.categoryBits = 0x10;
            b2Shape* shape = nullptr;
            if (trigger_type == "box") {
                auto* polyShape = new b2PolygonShape();
                polyShape->SetAsBox(trigger_width * 0.5f, trigger_height * 0.5f);
                shape = polyShape;
            } else if (trigger_type == "circle") {
                auto* circleShape = new b2CircleShape();
                circleShape->m_radius = trigger_radius;
                shape = circleShape;
            }
            assert(shape);
            fixture.isSensor = true;
            fixture.shape = shape;
            body->CreateFixture(&fixture);
            delete shape;
        }
    }*/
}

void RigidBody::onDestroy() {
    //PhysicsManager::getB2World().DestroyBody(body);
}

void PhysicsManager::startUp() {
    //getB2World().SetContactListener(&collisionDetector);
}

//void CollisionDetector::BeginContact(b2Contact* contact) {
//    auto fixtureA = contact->GetFixtureA();
//    auto fixtureB = contact->GetFixtureB();
//    auto actorA = reinterpret_cast<Actor*>(fixtureA->GetUserData().pointer);
//    auto actorB = reinterpret_cast<Actor*>(fixtureB->GetUserData().pointer);
//    if (!actorA || !actorB) return;
//    if ((fixtureA->GetFilterData().categoryBits & fixtureB->GetFilterData().categoryBits) == 0) return;
//    Collision collision;
//    collision.other = actorB;
//    collision.rVel = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
//    
//    if (fixtureA->IsSensor() && fixtureB->IsSensor()) {
//        collision.point = { -999.0f, -999.0f };
//        collision.normal = { -999.0f, -999.0f };
//        actorA->onTriggerEnter(collision);
//        collision.other = actorA;
//        actorB->onTriggerEnter(collision);
//    } else if (!fixtureA->IsSensor() && !fixtureB->IsSensor()) {
//        b2WorldManifold worldManifold;
//        contact->GetWorldManifold(&worldManifold);
//        collision.point = worldManifold.points[0];
//        collision.normal = worldManifold.normal;
//        actorA->onCollisionEnter(collision);
//        collision.other = actorA;
//        actorB->onCollisionEnter(collision);
//    }
//}

//void CollisionDetector::EndContact(b2Contact* contact) {
//    auto fixtureA = contact->GetFixtureA();
//    auto fixtureB = contact->GetFixtureB();
//    auto actorA = reinterpret_cast<Actor*>(fixtureA->GetUserData().pointer);
//    auto actorB = reinterpret_cast<Actor*>(fixtureB->GetUserData().pointer);
//    if (!actorA || !actorB) return;
//    Collision collision;
//    collision.other = actorB;
//    collision.point = { -999.0f,-999.0f };
//    collision.normal = { -999.0f,-999.0f };
//    collision.rVel = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
//
//    if ((fixtureA->GetFilterData().categoryBits & fixtureB->GetFilterData().categoryBits) == 0) return;
//    if (fixtureA->IsSensor() && fixtureB->IsSensor()) {
//        if (fixtureA->GetFilterData().categoryBits)
//        actorA->onTriggerExit(collision);
//        collision.other = actorA;
//        actorB->onTriggerExit(collision);
//    } else if (!fixtureA->IsSensor() && !fixtureB->IsSensor()) {
//        actorA->onCollisionExit(collision);
//        collision.other = actorA;
//        actorB->onCollisionExit(collision);
//    }
//}

//luabridge::LuaRef HitResult::getTable() {
//    auto ref = luabridge::newTable(g_context.componentSystem.luaState.get());
//    ref["actor"] = actor;
//    ref["point"] = point;
//    ref["normal"] = normal;
//    ref["is_trigger"] = is_trigger;
//    return ref;
//}
//
//HitResult::HitResult(Actor* actor, const b2Vec2& point, const b2Vec2& normal, bool isTrigger, float fraction)
//    : actor(actor), point(point), normal(normal), is_trigger(isTrigger), fraction(fraction){}
//
}