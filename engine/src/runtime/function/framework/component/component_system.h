#pragma once

#include "runtime/resource/asset_manager.h"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "lua/lstate.h"
#include <unordered_set>

namespace Merak {
    extern const std::string componentPath;

    //class Component {
    //public:
    //    Component(const luabridge::LuaRef& ref);
    //    std::shared_ptr<luabridge::LuaRef> table;
    //    // std::unordered_map<std::string, luabridge::LuaRef> funcs;
    //    bool start = true;
    //    virtual ~Component();
    //    // forward all key-value pairs in data to the table
    //    void modify(const rapidjson::Value& data);
    //    void setElement(const char* key, const rapidjson::Value& value);
    //};

    class ComponentSystem {
    private:
        std::unordered_map<std::string, luabridge::LuaRef> loadedComponents;

        void loadLua(const std::string& compName);

    public:
        

        static inline std::string componentPath;

        static void getComponentPath();

        luabridge::LuaRef createComponent();

        using luaState_p = std::unique_ptr<lua_State, void(*)(lua_State*)>;
        luaState_p luaState;

        /**
         * @brief instantiate a component by name. Will load the parent component if not yet.
        */
        luabridge::LuaRef getComp(const rapidjson::Value& value, const std::string& compKey);

        luabridge::LuaRef loadComp(const std::string& name, const std::string& compKey);

        void injectLuaGlobal();

        void inheritTable(luabridge::LuaRef& instance, luabridge::LuaRef& parent);

        static void forwardElement(luabridge::LuaRef table, const std::string& name, const rapidjson::Value& value);

        void startUp();

        void cleanUp();

        ComponentSystem();
        ~ComponentSystem();
    };
}