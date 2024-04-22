#pragma once

#include "runtime/function/framework/world/world_manager.h"

#include <string>
#include <memory>

namespace Merak {
	class RigidBody {
	public:
		bool enabled = true;
		std::string key = "";
		std::string type = "Rigidbody";
		float width = 1.0f;
		float height = 1.0f;
		float radius = 0.5f;
		float friction = 0.3f;
		float bounciness = 0.3f;
		float trigger_width = 1.0f;
		float trigger_height = 1.0f;
		float trigger_radius = 0.5f;
		float x = 0.0f;
		float y = 0.0f;
		std::string body_type = "dynamic";
		std::string collider_type = "box";
		std::string trigger_type = "box";
		bool precise = true;
		float gravity_scale = 1.0f;
		float density = 1.0f;
		float angular_friction = 0.3f;
		// in degrees
		float rotation = 0.0f;
		bool has_collider = true;
		bool has_trigger = true;
		// b2Body* body = nullptr;
		Actor* actor = nullptr;
		/*std::unique_ptr<Merak::Actor> _Actor = nullptr;
		Actor* actor = nullptr;*/
		virtual ~RigidBody();
		void init();
		void onDestroy();
		RigidBody() = default;
	};

	//class CollisionDetector : public b2ContactListener {
	//	void BeginContact(b2Contact* contact) override;
	//	void EndContact(b2Contact* contact) override;
	//};

	//class HitResult {
	//public:
	//	Actor* actor = nullptr;
	//	b2Vec2 point;
	//	b2Vec2 normal;
	//	bool is_trigger;
	//	float fraction = 0.0f; // to sort in rayCastAll
	//	luabridge::LuaRef getTable();
	//	HitResult(Actor*, const b2Vec2&, const b2Vec2&, bool, float);
	//};

	class PhysicsManager {
	public:
		/*static b2World& getB2World();
		CollisionDetector collisionDetector;*/
		void tick();
		void startUp();
	};
}

