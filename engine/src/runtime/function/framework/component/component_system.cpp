#include "runtime/function/framework/component/component_system.h"
#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/global/global_context.h"
#include "runtime/function/framework/event/event_manager.h"
#include <thread>
//#include "box2d/box2d.h"
#include "runtime/core/math/math.h"

namespace {
    void CppLog(const std::string& message) {
        std::cout << message << '\n';
    }
    void CppLogErr(const std::string& message) {
        std::cerr << message << '\n';
    }
    Merak::Actor* FindActorByName(const std::string& name) {
        auto& ids = Merak::g_context.worldManager.getActorsOfName(name);
        if (ids.empty()) return nullptr;
        auto iter = Merak::g_context.worldManager.currentScene->actors.find(*ids.begin());
        if (iter != Merak::g_context.worldManager.currentScene->actors.end()) {
            return iter->second.get();
        }
        return Merak::g_context.worldManager.currentScene->actorsToAdd[*ids.begin()].get();
    }

    luabridge::LuaRef FindActorsOfName(const std::string& name) {
        auto& ids = Merak::g_context.worldManager.getActorsOfName(name);
        auto ans = luabridge::newTable(Merak::g_context.componentSystem.luaState.get());
        if (ids.empty()) return ans;
        uint32_t count = 1;
        for (const auto& id : ids) {
            auto iter = Merak::g_context.worldManager.currentScene->actors.find(id);
            ans[count++] = 
                iter != Merak::g_context.worldManager.currentScene->actors.end() ? 
                Merak::g_context.worldManager.currentScene->actors[id].get() : 
                Merak::g_context.worldManager.currentScene->actorsToAdd[id].get();
        }
        return ans;
    }

    void setCameraPos(float x, float y) {
        // Merak::g_context.renderSystem.camera.setPos({ x, y });
    }

    void setCameraZoom(float z) {
        // Merak::g_context.renderSystem.camera.setZoom(z);
    }

    void loadNewScene(const std::string& name) {
        Merak::g_context.worldManager.newScene = name;
    }

    std::string getCurrentScene() {
        return Merak::g_context.worldManager.currentScene->name;
    }

    void dontDestroyActor(Merak::Actor& actor) {
        Merak::g_context.worldManager.keepActor(actor);
    }

    void draw3D(const std::string& model, const std::string& texture, luabridge::LuaRef modelMat) {
        glm::mat4 mat{};
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                mat[j][i] = modelMat[i + 1][j + 1].cast<float>();
            }
        }
        Merak::g_context.renderSystem.addDrawCall(model, texture, std::move(mat));
    }

    /*class Cast : public b2RayCastCallback {
    public:
        b2Fixture* fixture = nullptr;
        b2Vec2 hitPoint;
        b2Vec2 hitNormal;
        float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override {
            if (!fixture->GetUserData().pointer) {
                return -1.0f;
            }
            this->fixture = fixture;
            hitPoint = point;
            hitNormal = normal;
            return fraction;
        };
    };

    luabridge::LuaRef rayCast(const b2Vec2& pos, b2Vec2 dir, const float& dist) {
        auto cast = std::make_unique<Cast>();
        dir.Normalize();
        Merak::PhysicsManager::getB2World().RayCast(cast.get(), pos, pos + b2Vec2(dir.x * dist, dir.y * dist));
        if (cast->fixture) {
            Merak::HitResult res(
                reinterpret_cast<Merak::Actor*>(cast->fixture->GetUserData().pointer), 
                cast->hitPoint,
                cast->hitNormal,
                cast->fixture->IsSensor(),
                0.0f);
            return res.getTable();
        } else {
            return luabridge::LuaRef(Merak::g_context.componentSystem.luaState.get());
        }
    }

    class CastAll : public b2RayCastCallback {
    public:
        std::vector<Merak::HitResult> vec;
        float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override {
            if (!fixture->GetUserData().pointer) {
                return -1.0f;
            }
            vec.emplace_back(reinterpret_cast<Merak::Actor*>(fixture->GetUserData().pointer), point, normal, fixture->IsSensor(), fraction);
            return 1.0f;
        };
        luabridge::LuaRef getTable() {
            std::sort(vec.begin(), vec.end(), [](const Merak::HitResult& lhs, const Merak::HitResult& rhs) {
                return lhs.fraction < rhs.fraction;
                });
            auto ref = luabridge::newTable(Merak::g_context.componentSystem.luaState.get());
            for (int i = 0; i < vec.size(); i++) {
                ref[i + 1] = vec[i].getTable();
            }
            return ref;
        }
    };

    luabridge::LuaRef rayCastAll(const b2Vec2& pos, const b2Vec2& dir, const float& dist) {
        auto cast = std::make_unique<CastAll>();
        Merak::PhysicsManager::getB2World().RayCast(cast.get(), pos, pos + b2Vec2(dir.x * dist, dir.y * dist));
        return cast->getTable();
    }*/
};

namespace Merak {
  
    void ComponentSystem::loadLua(const std::string& compName) {
        auto path = componentPath + compName + ".lua";
        if (!std::filesystem::exists(path)) {
            std::cout << "error: failed to locate component " << compName;
            exit(0);
        }
        // visual studio profiler error happens here:
        // it can be traced to luaD_rawrunprotected()
        if (luaL_dofile(luaState.get(), path.c_str()) != LUA_OK) {
            std::cout << "problem with lua file " << compName;
            exit(0);
        }
    }

    void ComponentSystem::getComponentPath() {
        componentPath = resPath + "/component_types/";
    }

    luabridge::LuaRef ComponentSystem::loadComp(const std::string& name, const std::string& compKey) {
        auto comp = createComponent();
        if (loadedComponents.find(name) == loadedComponents.end()) {
            loadLua(name);
            loadedComponents.insert({ name, luabridge::getGlobal(luaState.get(), name.c_str()) });
        }
        auto parentComp = loadedComponents.at(name);
        inheritTable(comp, parentComp);
        comp["key"] = compKey;
        comp["type"] = name;
        return comp;
    }

    luabridge::LuaRef ComponentSystem::getComp(const rapidjson::Value& value, const std::string& compKey) {
        std::string compName{ value["type"].GetString() };
        auto comp = loadComp(compName, compKey);

        for (auto iter = value.MemberBegin(); iter != value.MemberEnd(); ++iter) {
            if (strcmp(iter->name.GetString(), "type") == 0) continue;
            forwardElement(comp, iter->name.GetString(), iter->value);
        }
        return comp;
    }

    void ComponentSystem::inheritTable(luabridge::LuaRef& instance, luabridge::LuaRef& parent) {
        luabridge::LuaRef meta = luabridge::newTable(luaState.get());
        meta["__index"] = parent;

        instance.push(luaState.get());
        meta.push(luaState.get());
        lua_setmetatable(luaState.get(), -2);
        lua_pop(luaState.get(), 1);
    }

    void ComponentSystem::injectLuaGlobal() {
        luabridge::getGlobalNamespace(luaState.get())
            .beginNamespace("Debug")
            .addFunction("Log", CppLog)
            .addFunction("LogError", CppLogErr)
            .endNamespace();

        luabridge::getGlobalNamespace(luaState.get())
            .beginClass<Actor>("Actor")
            .addFunction("GetName", &Actor::getName)
            .addFunction("GetID", &Actor::getID)
            .addFunction("GetComponentByKey", &Actor::getCompByKey)
            .addFunction("GetComponent", &Actor::getCompByType)
            .addFunction("GetComponents", &Actor::getCompOfType)
            .addFunction("AddComponent", &Actor::addComp)
            .addFunction("RemoveComponent", &Actor::removeComp)
            .endClass();

        luabridge::getGlobalNamespace(luaState.get())
            .beginNamespace("Actor")
            .addFunction("Find", FindActorByName)
            .addFunction("FindAll", FindActorsOfName)
            .addFunction("Instantiate", static_cast<Actor&(*)(const std::string&)>([](const std::string& tempName) -> Actor& {
                return *g_context.worldManager.currentScene->createActor(tempName);
            }))
            .addFunction("Destroy", static_cast<void(*)(Actor&)>([](Actor& actor){
                    g_context.worldManager.currentScene->destroyActor(actor);
                }))
            .endNamespace();

        luabridge::getGlobalNamespace(luaState.get())
            .beginNamespace("Application")
            .addFunction("Quit", static_cast<void (*)()>([]() { exit(0); }))
            .addFunction("Sleep", static_cast<void (*)(int)>([](int t) { 
                    std::this_thread::sleep_for(std::chrono::milliseconds(t));
                }))
            /*.addFunction("GetFrame", static_cast<int (*)()>([]() {
                    return Helper::GetFrameNumber();
                }))*/
            .addFunction("OpenURL", static_cast<void (*)(const std::string&)>([](const std::string& url) {
#ifdef _WIN32
                    std::system(("start " + url).c_str());
#elif __APPLE__
                    std::system(("open " + url).c_str());
#else
                    std::system(("xdg-open " + url).c_str());
#endif 
                }))
            .endNamespace();

        luabridge::getGlobalNamespace(luaState.get())
            .beginClass<glm::vec2>("vec2")
            .addProperty("x", &glm::vec2::x)
            .addProperty("y", &glm::vec2::y)
            .endClass();

        luabridge::getGlobalNamespace(luaState.get())
            .beginNamespace("Input")
            .addFunction("GetKey", InputSystem::getKey)
            .addFunction("GetKeyDown", InputSystem::getKeyDown)
            .addFunction("GetKeyUp", InputSystem::getKeyUp)
            .addFunction("GetMousePosition", InputSystem::getMousePos)
            .addFunction("GetMouseButton", InputSystem::getMouseButton)
            .addFunction("GetMouseButtonDown", InputSystem::getMouseButtonDown)
            .addFunction("GetMouseButtonUp", InputSystem::getMouseButtonUp)
            .addFunction("GetMouseScrollDelta", static_cast<float(*)()>([]() {
                    return InputSystem::mouseScrollY;
                }))
            .endNamespace();

        luabridge::getGlobalNamespace(luaState.get())
            .beginNamespace("Scene")
            .addFunction("Load", loadNewScene)
            .addFunction("GetCurrent", getCurrentScene)
            .addFunction("DontDestroy", dontDestroyActor)
            .endNamespace();

        luabridge::getGlobalNamespace(luaState.get())
            .beginNamespace("Model")
            .addFunction("Draw", draw3D)
            .endNamespace();

        luabridge::getGlobalNamespace(luaState.get())
            .beginNamespace("Event")
            .addFunction("Publish", static_cast<void(*)(const std::string&, luabridge::LuaRef)>(
                [](const std::string& event, luabridge::LuaRef info) {
                    g_context.eventManager.publish(event, info);
                }))
            .addFunction("Subscribe", static_cast<void(*)(const std::string&, luabridge::LuaRef, luabridge::LuaRef)>(
                [](const std::string& event, luabridge::LuaRef comp, luabridge::LuaRef func) {
                    g_context.eventManager.subscribe(event, comp, func);
                }))
            .addFunction("Unsubscribe", static_cast<void(*)(const std::string&, luabridge::LuaRef, luabridge::LuaRef)>(
                [](const std::string& event, luabridge::LuaRef comp, luabridge::LuaRef func) {
                    g_context.eventManager.unsubscribe(event, comp, func);
                }))
            .endNamespace();
    }

    void ComponentSystem::startUp() {
        luaState = luaState_p(luaL_newstate(
        ), 
            [](lua_State* pState){
                lua_close(pState);
            });
        luaL_openlibs(luaState.get());
        injectLuaGlobal();
        getComponentPath();
    }

    void ComponentSystem::cleanUp() {
        loadedComponents.clear();
    }

    luabridge::LuaRef ComponentSystem::createComponent() {
        auto newTable = luabridge::newTable(luaState.get());
        newTable["enabled"] = true;
        return newTable;
    }

    void ComponentSystem::forwardElement(luabridge::LuaRef table, const std::string& name, const rapidjson::Value& value) {
        if (value.IsString()) {
            table[name] = value.GetString();
        }
        else if (value.IsInt())
           table[name] = value.GetInt();
        else if (value.IsFloat())
           table[name] = value.GetFloat();
        else if (value.IsBool()) {
            table[name] = value.GetBool();
        }
    }  

    ComponentSystem::ComponentSystem() : luaState(nullptr, lua_close){}
    ComponentSystem::~ComponentSystem() {
        cleanUp();
    }
}
