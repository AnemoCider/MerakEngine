#pragma once

#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "lua/lstate.h"
#include <unordered_map>
#include <string>
#include <vector>

namespace Merak {
	class EventManager {
	private:
		std::unordered_map<std::string, uint32_t> ids;
		using Subscriber = std::pair<luabridge::LuaRef, luabridge::LuaRef>;
		std::unordered_map<std::string, std::map<uint32_t, Subscriber>> events;
		std::unordered_map<std::string, std::vector<Subscriber>> subscribers;
		std::unordered_map<std::string, std::vector<Subscriber>> unsubscribers;
	public:
		void publish(const std::string&, luabridge::LuaRef);
		void subscribe(const std::string&, luabridge::LuaRef, luabridge::LuaRef);
		void unsubscribe(const std::string&, luabridge::LuaRef, luabridge::LuaRef);
		void process();
	};
};