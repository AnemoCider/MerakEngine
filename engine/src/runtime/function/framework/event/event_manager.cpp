#include "runtime/function/framework/event/event_manager.h"
#include "runtime/function/global/global_context.h"
#include <algorithm>

namespace {
	void reportError(const std::string& actorName, const luabridge::LuaException& e) {
		std::string mes = e.what();
		std::replace(mes.begin(), mes.end(), '\\', '/');
		std::cout << "\033[31m" << actorName << " : " << e.what() <<
			"\033[0m" << '\n';
	}
};

namespace Merak {
	void EventManager::publish(const std::string& event, luabridge::LuaRef eventInfo) {
		for (auto& [_, sub] : events[event]) {
			if (!sub.second.isNil()) {
				try {
					sub.second(sub.first, eventInfo);
				}
				catch (luabridge::LuaException& e) {
					reportError(sub.first["actor"].tostring(), e);
				}
			}
		}
	};

	void EventManager::subscribe(const std::string& event, luabridge::LuaRef comp, luabridge::LuaRef func) {
		subscribers[event].push_back({ comp, func });
	}

	void EventManager::unsubscribe(const std::string& event, luabridge::LuaRef comp, luabridge::LuaRef func) {
		unsubscribers[event].push_back({ comp, func });
	}

	void EventManager::process() {
		for (auto& pair : subscribers) {
			for (auto& sub : pair.second) {
				if (sub.first["_events"].isNil()) {
					sub.first["_events"] = luabridge::newTable(g_context.componentSystem.luaState.get());
				}
				auto id = ids[pair.first]++;
				sub.first["_events"][pair.first] = id;
				events[pair.first].emplace(id, sub);
			}
		}
		subscribers.clear();

		for (auto& pair : unsubscribers) {
			for (auto& sub : pair.second) {
				events.at(pair.first).erase(sub.first["_events"][pair.first].cast<uint32_t>());
			}
		}
		unsubscribers.clear();
	}

	
}