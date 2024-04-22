#pragma once

//#include "SDL2/SDL.h"
//#include "SDL_image/SDL_image.h"
#include <vector>
#include <array>
#include <memory>
#include <unordered_set>
#include <set>
#include <glm/glm.hpp>
#include <functional>
#include <string>
#include <optional>
#include "runtime/resource/asset_manager.h"
#include "runtime/function/framework/component/component_system.h"
//#include "box2d/box2d.h"

static const int STRIDE  = 100;

namespace Merak {

	struct Transform {
		glm::vec2 position = {0.0f, 0.0f};
		// clockwise rotation, in degrees
		glm::dvec2 scale = {1.0f, 1.0f};
		double rotation = 0.0f;
		std::array<std::optional<double>, 2> offset;
	};

	/*struct Collision;*/

	struct Actor {
		std::string actor_name = "";
		uint32_t id;

		bool keepInNext = false;

		std::vector<luabridge::LuaRef> componentsToAdd;
		std::unordered_set<std::string> componentsToRemove;

		std::map<std::string, luabridge::LuaRef> components;
		std::unordered_map<std::string, std::set<std::string>> compsByType;

		std::set<std::string> onStartComponents;
		std::set<std::string> onUpdateComponents;
		std::set<std::string> onLateUpdateComponents;
		/*std::set<std::string> onCollisionEnterComponents;
		std::set<std::string> onCollisionExitComponents;
		std::set<std::string> onTriggerEnterComponents;
		std::set<std::string> onTriggerExitComponents;*/

		void loadProps(const rapidjson::Value& a);

		void start();
		void update();
		void lateUpdate();
		void destroy();

		const char* getName();
		uint32_t getID();
		luabridge::LuaRef getCompByKey(const std::string& key);
		luabridge::LuaRef getCompByType(const std::string& type);
		luabridge::LuaRef getCompOfType(const std::string& type);

		/*void onCollisionEnter(const Collision&);
		void onCollisionExit(const Collision&);
		void onTriggerEnter(const Collision&);
		void onTriggerExit(const Collision&);*/

		// add a component to the actor.
		// note that all life cycle functions are not added until the end of frame,
		// to avoid them being called in the current cycle.
		// only component by type is added at once
		luabridge::LuaRef addComp(const std::string& type);

		// inherit properties from another actor.
		// remember to manually set the id!
		void inheritFromTemplate(Merak::Actor&);

		/* Add component's key to the life cycle function sets*/
		void addComponentFunctions(luabridge::LuaRef);
		void removeComponentFunctions(const std::string& key);

		void removeComp(luabridge::LuaRef comp);
		void clear();

	};

	//struct Collision {
	//	Actor* other;
	//	b2Vec2 point;
	//	b2Vec2 rVel;
	//	b2Vec2 normal;
	//};

	struct ActorUpdateComp {
		bool operator()(const Actor* lhs, const Actor* rhs) const {
			return lhs->id < rhs->id;
		}
		bool operator()(const std::shared_ptr<Actor>& lhs, const std::shared_ptr<Actor>& rhs) const {
			return lhs->id < rhs->id;
		}
	};

	/*struct RenderInfo {
		Merak::Actor* actor;
		SDL_Rect rect;
		SDL_Point pivot;
		RenderInfo(Merak::Actor* a, const SDL_Rect& rect, const SDL_Point& p) :
			actor(a), rect(rect), pivot(p) {}
	};*/

	struct ivec2Hash {
		uint64_t operator()(const glm::ivec2& pos) const {
			return (uint64_t)(pos.x) << 32 | (uint32_t)(pos.y);
		}
	};

	class Scene {
	public:
		std::string name;

		// we may need to query a newly spawned actor by id
		// when it has not been added to actors
		std::unordered_map<uint32_t, std::shared_ptr<Actor>> actorsToAdd;
		std::vector<uint32_t> actorsToRemove;
		std::map<uint32_t, std::shared_ptr<Actor>> actors;
		std::unordered_map<std::string, std::set<uint32_t>> actorsByName;

		bool isOnStart = true;

		static int getDistance(const glm::ivec2& pos1, const glm::ivec2& pos2);
		void clear();
		std::unique_ptr<Actor> ptr;
		// create an actor from template
		Actor* createActor(const std::string& tempName);
		void destroyActor(Actor&);
		
	};

	class WorldManager {
	public:
		std::string templatePath;

		std::unordered_map<std::string, Actor> templates;

		uint32_t id;
		uint32_t addCompCount;
		// std::unordered_map<std::string, std::shared_ptr<Scene>> scenes;

		std::optional<std::string> newScene;
		std::vector<std::shared_ptr<Actor>> initActorsInNewScene;

		std::shared_ptr<Scene> currentScene = { nullptr, [](Scene* scene) {
				if (scene) scene->clear();
			} };

		void startUp();
		void cleanUp();
		~WorldManager();

		void executeForAll(const std::vector<std::shared_ptr<Actor>>& vec, const std::string& func);

		void processNewScene();
		void processManipulation();

		void tick();

		std::set<uint32_t>& getActorsOfName(const std::string& name);

		// keep this actor in the new scene
		void keepActor(Actor&);

		/**
		*	@return index of the player in the vector. If not present, return -1.
		*/
		void loadActors(const rapidjson::Document& sceneDoc);

		/*
			@return -1 if fail to load, 0 otherwise
		*/
		int loadScene(const std::string& levelName);

		// create the template if not already
		Actor& getTemplate(const std::string&);
	};
}

