#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/global/global_context.h"

namespace {
	void reportError(const std::string& actorName, const luabridge::LuaException& e) {
		std::string mes = e.what();
		std::replace(mes.begin(), mes.end(), '\\', '/');
		std::cout << "\033[31m" << actorName << " : " << e.what() <<
			"\033[0m" << '\n';
	}
};

void Merak::WorldManager::processNewScene() {
	if (newScene.has_value()) {
		// currentScene->clear(); called by unique ptr deleter
		loadScene(newScene.value());
		newScene.reset();
		for (auto& p : initActorsInNewScene) {
			p->keepInNext = false;
			currentScene->actors.emplace(p->id, p);
			auto iter = currentScene->actorsByName.find(p->actor_name);
			if (iter != currentScene->actorsByName.end()) iter->second.insert(p->id);
			else {
				currentScene->actorsByName.insert({ p->actor_name, {} });
				currentScene->actorsByName.at(p->actor_name).insert(p->id);
			}
		}
		initActorsInNewScene.clear();
	}
}

void Merak::WorldManager::startUp() {
	templatePath = resPath + "/actor_templates";
	if (loadScene(g_context.assetManager.gameConfig["initial_scene"].GetString()) == -1) {
		std::cout << "error: scene " << g_context.assetManager.gameConfig["initial_scene"].GetString() << " is missing";
		exit(0);
	}
}

void Merak::WorldManager::cleanUp() {
	// shared pointer will automatically do the cleanUp
	templates.clear();
	currentScene = nullptr;
}

Merak::WorldManager::~WorldManager() {
	cleanUp();
}

void Merak::WorldManager::executeForAll(const std::vector<std::shared_ptr<Actor>>& vec, const std::string& func) {
	for (auto& actor : vec) {
		for (auto& [name, comp] : actor->components) {
			if (!comp["enabled"]) continue;
			auto f = comp[func];
			if (!f.isNil()) {
				try {
					f(comp);
				}
				catch (luabridge::LuaException& e) {
					reportError(actor->actor_name, e);
				}
			}
		}
	}
}

void Merak::WorldManager::processManipulation() {
	for (auto& p : currentScene->actorsToAdd) {
		currentScene->actors.insert(currentScene->actors.end(), { p.first, std::move(p.second) });
	}
	currentScene->actorsToAdd.clear();

	for (auto& id : currentScene->actorsToRemove) {
		currentScene->actors.erase(id);
	}
	currentScene->actorsToRemove.clear();
	
	for (auto& [_, actor] : currentScene->actors) {
		// Remove should be after creation
		// because user can remove immediately after creation
		for (auto& comp : actor->componentsToAdd) {
			actor->components.emplace(comp["key"].cast<std::string>(), comp);
			actor->addComponentFunctions(comp);
		}
		actor->componentsToAdd.clear();

		for (auto& comp : actor->componentsToRemove) {
			actor->components.erase(comp);
			actor->removeComponentFunctions(comp);
		}
		actor->componentsToRemove.clear();
	}
}

void Merak::WorldManager::tick() {
	assert(currentScene);
	for (auto& [_, actor] : currentScene->actors) {
		actor->start();
	}
	for (auto& [_, actor] : currentScene->actors) {
		actor->update();
	}

	for (auto& [_, actor] : currentScene->actors) {
		actor->lateUpdate();
	}
}

std::set<uint32_t>& Merak::WorldManager::getActorsOfName(const std::string& name) {
	return currentScene->actorsByName[name];
}

void Merak::WorldManager::keepActor(Actor& actor) {
	currentScene->actors[actor.id]->keepInNext = true;
	initActorsInNewScene.push_back(currentScene->actors[actor.id]);
}

int Merak::WorldManager::loadScene(const std::string& levelName) {
	std::string scenePath = resPath + "/scenes/" + levelName + ".scene";
	if (!std::filesystem::exists(scenePath)) {
		return -1;
	}
	if (currentScene) {
		currentScene->clear();
	}
	currentScene = std::make_shared<Scene>();
	currentScene->name = levelName;
	rapidjson::Document sceneConfig{};
	AssetManager::ReadJsonFile(scenePath, sceneConfig);
	loadActors(sceneConfig);
	// setColliders();	
	/*if (player.exists() && player.p->canTrigger()) {
		setTriggers(player.p);
	}*/
	return 0;
}

Merak::Actor& Merak::WorldManager::getTemplate(const std::string& tempName) {
	// TODO: insert return statement here
	if (templates.find(tempName) == templates.end()) {
		auto templateFile = templatePath + '/' + tempName + ".template";
		if (!std::filesystem::exists(templatePath) || !std::filesystem::exists(templateFile)) {
			std::cout << "error: template " << tempName << " is missing";
			exit(0);
		}
		rapidjson::Document templateDoc{};
		AssetManager::ReadJsonFile(templateFile, templateDoc);
		templates[tempName].loadProps(templateDoc);
	}
	return templates[tempName];
}

void Merak::WorldManager::loadActors(const rapidjson::Document& sceneDoc) {
	for (auto& a : sceneDoc["actors"].GetArray()) {
		auto act = std::make_shared<Actor>();
		if (a.HasMember("template")) {
			act->inheritFromTemplate(getTemplate(a["template"].GetString()));
		}
		act->id = id++;
		act->loadProps(a);
		for (auto& [_, comp] : act->components) {
			comp["actor"] = act.get();
		}
		currentScene->actors.insert(currentScene->actors.end(), {act->id,act});
		currentScene->actorsByName[act->actor_name].insert(act->id);
	}
}

Merak::Actor* Merak::Scene::createActor(const std::string& tempName) {
	auto act = std::make_shared<Actor>();
	act->inheritFromTemplate(g_context.worldManager.getTemplate(tempName));
	act->id = g_context.worldManager.id++;
	for (auto& [_, comp] : act->components) {
		comp["actor"] = act.get();
	}
	actorsToAdd.insert({ act->id, act });
	actorsByName[act->actor_name].insert(act->id);
	return act.get();
}

void Merak::Scene::destroyActor(Actor& actor) {
	actorsToRemove.push_back(actor.id);
	actorsByName.at(actor.actor_name).erase(actor.id);
	auto iter = actors.find(actor.id);
	// the actor may have just been created and not added to the list
	if (iter != actors.end()) {
		actor.destroy();
		for (auto& comp : iter->second->components) {
			comp.second["enabled"] = false;
		}
	}
}

int Merak::Scene::getDistance(const glm::ivec2 & pos1, const glm::ivec2 & pos2) {
	return std::abs(pos1.x - pos2.x) + std::abs(pos1.y - pos2.y);
}

void Merak::Scene::clear() {
	for (auto& p : actors) {
		p.second->destroy();
	}
	actors.clear();
	actorsByName.clear();
	// scene->activeActors.clear();
	// positionMap.clear();
}

void Merak::Actor::loadProps(const rapidjson::Value& a) {
	if (a.HasMember("name")) actor_name = a["name"].GetString();
	if (a.HasMember("components")) {
		/*std::vector<std::string> onStart;
		std::vector<std::string> onUpdate;
		std::vector<std::string> onLateUpdate;*/
		for (auto it = a["components"].MemberBegin(); it != a["components"].MemberEnd(); it++) {
			auto key = it->name.GetString();
			auto iter = components.find(key);
			if (iter == components.end()) {
				auto newComp = g_context.componentSystem.getComp(it->value, it->name.GetString());
				components.emplace(
					key, newComp
				);
				compsByType[it->value["type"].GetString()].insert(key);
				// newComp["type"] = it->value["type"].GetString();
				// std::cout << newComp["type"] << ' ' << newComp["OnStart"] << '\n';
				/*if (!newComp["OnStart"].isNil()) onStart.push_back(key);
				if (!newComp["OnUpdate"].isNil()) onUpdate.push_back(key);
				if (!newComp["OnLateUpdate"].isNil()) onLateUpdate.push_back(key);*/
				addComponentFunctions(newComp);
			}
			else {
				for (auto memIter = it->value.MemberBegin(); memIter != it->value.MemberEnd(); memIter++) {
				    ComponentSystem::forwardElement(iter->second, memIter->name.GetString(), memIter->value);
				}
			}
		}
		/*std::sort(onStart.begin(), onStart.end());
		for (auto& s : onStart) {
			onStartComponents.insert(onStartComponents.end(), s);
		}
		std::sort(onUpdate.begin(), onUpdate.end());
		for (auto& s : onUpdate) {
			onUpdateComponents.insert(onUpdateComponents.end(), s);
		}
		std::sort(onLateUpdate.begin(), onLateUpdate.end());
		for (auto& s : onLateUpdate) {
			onLateUpdateComponents.insert(onLateUpdateComponents.end(), s);
		}*/
	}
}

void Merak::Actor::start() {
	if (onStartComponents.empty()) return;
	for (auto& str : onStartComponents) {
		auto& comp = components.at(str);
		if (!comp["enabled"]) continue;
		auto f = comp["OnStart"];
		try {
			f(comp);
		}
		catch (luabridge::LuaException& e) {
			reportError(actor_name, e);
		}
	}
	onStartComponents.clear();
}

void Merak::Actor::update() {
	if (onUpdateComponents.empty()) return;
	for (auto& str : onUpdateComponents) {
		auto& comp = components.at(str);
		if (!comp["enabled"]) continue;
		auto f = comp["OnUpdate"];
		try {
			f(comp);
		}
		catch (luabridge::LuaException& e) {
			reportError(actor_name, e);
		}
	}
}

void Merak::Actor::lateUpdate() {
	if (onLateUpdateComponents.empty()) return;
	for (auto& str : onLateUpdateComponents) {
		auto& comp = components.at(str);
		if (!comp["enabled"]) continue;
		auto f = comp["OnLateUpdate"];
		try {
			f(comp);
		}
		catch (luabridge::LuaException& e) {
			reportError(actor_name, e);
		}
	}
}

void Merak::Actor::destroy() {
	for (auto& pair : components) {
		auto& comp = pair.second;
		auto f = comp["OnDestroy"];
		if (f.isNil()) continue;
		try {
			f(comp);
		}
		catch (luabridge::LuaException& e) {
			reportError(actor_name, e);
		}
	}
}

const char* Merak::Actor::getName() {
	return actor_name.c_str();
}

uint32_t Merak::Actor::getID() {
	return id;
}

luabridge::LuaRef Merak::Actor::getCompByKey(const std::string& key) {
	auto iter = components.find(key);
	if (iter == components.end() || componentsToRemove.find(key) != componentsToRemove.end()) 
		return luabridge::LuaRef(g_context.componentSystem.luaState.get());
	return iter->second;
	
}

luabridge::LuaRef Merak::Actor::getCompByType(const std::string& type) {
	auto iter = compsByType.find(type);
	if (iter == compsByType.end() || iter->second.empty()) return luabridge::LuaRef(g_context.componentSystem.luaState.get());
	return components.at(*iter->second.begin());
}

luabridge::LuaRef Merak::Actor::getCompOfType(const std::string& type) {
	auto iter = compsByType.find(type);
	auto ans = luabridge::newTable(g_context.componentSystem.luaState.get());
	if (iter == compsByType.end()) return ans;
	uint32_t count = 1;
	for (const auto& key : iter->second) {
		ans[count++] = components.at(key);
	}
	return ans;
}

//void Merak::Actor::onCollisionEnter(const Collision& collision) {
//	for (auto& str : onCollisionEnterComponents) {
//		auto& comp = components.at(str);
//		if (!comp["enabled"]) continue;
//		auto f = comp["OnCollisionEnter"];
//		try {
//			f(comp, collision);
//		}
//		catch (luabridge::LuaException& e) {
//			reportError(actor_name, e);
//		}
//	}
//}

//void Merak::Actor::onCollisionExit(const Collision& collision) {
//	for (auto& str : onCollisionExitComponents) {
//		auto& comp = components.at(str);
//		if (!comp["enabled"]) continue;
//		auto f = comp["OnCollisionExit"];
//		try {
//			f(comp, collision);
//		}
//		catch (luabridge::LuaException& e) {
//			reportError(actor_name, e);
//		}
//	}
//}
//
//void Merak::Actor::onTriggerEnter(const Collision& collision) {
//	for (auto& str : onTriggerEnterComponents) {
//		auto& comp = components.at(str);
//		if (!comp["enabled"]) continue;
//		auto f = comp["OnTriggerEnter"];
//		try {
//			f(comp, collision);
//		}
//		catch (luabridge::LuaException& e) {
//			reportError(actor_name, e);
//		}
//	}
//}
//
//void Merak::Actor::onTriggerExit(const Collision& collision) {
//	for (auto& str : onTriggerExitComponents) {
//		auto& comp = components.at(str);
//		if (!comp["enabled"]) continue;
//		auto f = comp["OnTriggerExit"];
//		try {
//			f(comp, collision);
//		}
//		catch (luabridge::LuaException& e) {
//			reportError(actor_name, e);
//		}
//	}
//}


luabridge::LuaRef Merak::Actor::addComp(const std::string& type) {
	auto key = "r" + std::to_string(g_context.worldManager.addCompCount++);
	auto newComp = g_context.componentSystem.loadComp(type, key);
	newComp["actor"] = *this;
	componentsToAdd.push_back(
		newComp
	);
	compsByType[type].insert(key);
	return newComp;
}

void Merak::Actor::inheritFromTemplate(Merak::Actor& temp) {
	for (auto& [key, comp] : temp.components) {
		components.emplace(key, g_context.componentSystem.createComponent());
		g_context.componentSystem.inheritTable(components.at(key), comp);
	}
	actor_name = temp.actor_name;
	compsByType = temp.compsByType;
	onStartComponents = temp.onStartComponents;
	onUpdateComponents = temp.onUpdateComponents;
	onLateUpdateComponents = temp.onLateUpdateComponents;
	/*onCollisionEnterComponents = temp.onCollisionEnterComponents;
	onCollisionExitComponents = temp.onCollisionExitComponents;
	onTriggerEnterComponents = temp.onTriggerEnterComponents;
	onTriggerExitComponents = temp.onTriggerExitComponents;*/
}

void Merak::Actor::addComponentFunctions(luabridge::LuaRef comp) {
	auto key = comp["key"].cast<std::string>();
	if (!comp["OnStart"].isNil())
		onStartComponents.insert(key);
	if (!comp["OnUpdate"].isNil())
		onUpdateComponents.insert(key);
	if (!comp["OnLateUpdate"].isNil())
		onLateUpdateComponents.insert(key);
	/*if (!comp["OnCollisionEnter"].isNil())
		onCollisionEnterComponents.insert(key);
	if (!comp["OnCollisionExit"].isNil())
		onCollisionExitComponents.insert(key);
	if (!comp["OnTriggerEnter"].isNil())
		onTriggerEnterComponents.insert(key);
	if (!comp["OnTriggerExit"].isNil())
		onTriggerExitComponents.insert(key);*/
}

void Merak::Actor::removeComponentFunctions(const std::string& key) {
	onStartComponents.erase(key);
	onUpdateComponents.erase(key);
	onLateUpdateComponents.erase(key);
	/*onCollisionEnterComponents.erase(key);
	onCollisionExitComponents.erase(key);
	onTriggerEnterComponents.erase(key);
	onTriggerExitComponents.erase(key);*/
}

void Merak::Actor::removeComp(luabridge::LuaRef comp) {
	comp["enabled"] = false;
	auto key = comp["key"].cast<std::string>();
	componentsToRemove.insert(key);
	compsByType[comp["type"].cast<std::string>()].erase(key);
	auto f = comp["OnDestroy"];
	if (!f.isNil()) {
		try {
			f(comp);
		}
		catch (luabridge::LuaException& e) {
			reportError(actor_name, e);
		}
	}
}

void Merak::Actor::clear() {
	componentsToAdd.clear();
	// componentsToRemove;
	components.clear();
	onStartComponents.clear();
	onUpdateComponents.clear();
	onLateUpdateComponents.clear();
	/*onCollisionEnterComponents.clear();
	onCollisionExitComponents.clear();
	onTriggerEnterComponents.clear();
	onTriggerExitComponents.clear();*/
}

