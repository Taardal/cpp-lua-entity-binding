#include <lua.hpp>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

#if 1
#define println(x, ...) printf(x, ##__VA_ARGS__); printf("\n")
#else
#define println(x, ...)
#endif

extern void printLua(lua_State* L, const std::string& tag = "NOTICE ME");

struct ScriptComponent {
    std::string Type;
};

struct TransformComponent {
    float X = 0.0;
    float Y = 0.0;
    float Z = 0.0;

    TransformComponent() = default;

    TransformComponent(const TransformComponent&) = default;
};

struct Entity {
    std::string Id;
    ScriptComponent ScriptComponent;
    TransformComponent TransformComponent;
};

struct Scene {
    std::unordered_map<std::string, Entity> Entities;
    std::set<std::string> Types;
};

struct EntityBinding {
    std::string entityId;
    Entity* entity;

    EntityBinding(const std::string& entityId, Entity* entity) : entityId(entityId), entity(entity) {
        println("Created binding for entity with id [%s]", entityId.c_str());
    }

    ~EntityBinding() {
        println("Destroyed binding for entity with id [%s]", entityId.c_str());
    }

    void* getComponent(const std::string& componentType) const {
        if (componentType == "ScriptComponent") {
            return &entity->ScriptComponent;
        }
        if (componentType == "TransformComponent") {
            return &entity->TransformComponent;
        }
        return nullptr;
    }

    bool hasComponent(const std::string& componentType) const {
        if (componentType == "ScriptComponent") {
            return true;
        }
        if (componentType == "TransformComponent") {
            return true;
        }
        return false;
    }

    static int newIndex(lua_State* L) {
        constexpr int bottomOfLuaStackIndex = 1;
        int userdataIndex = bottomOfLuaStackIndex;
        int keyIndex = userdataIndex + 1;
        int valueIndex = keyIndex + 1;
        std::string key = lua_tostring(L, keyIndex);

        bool indexFound = false;
        if (key == "entityId") {
            const char* value = lua_tostring(L, valueIndex);
            auto* binding = (EntityBinding*) lua_touserdata(L, userdataIndex);
            binding->entityId = std::string(value);
            indexFound = true;
        }
        if (!indexFound) {
            // Key does not exist on C++ class, attempt to set key=value on lua table
            lua_getuservalue(L, userdataIndex);
            lua_pushvalue(L, keyIndex);
            lua_pushvalue(L, valueIndex);
            lua_settable(L, -3);
        }
        return 1;
    }

    static int index(lua_State* L) {
        constexpr int bottomOfLuaStackIndex = 1;
        int userdataIndex = bottomOfLuaStackIndex;
        int keyIndex = userdataIndex + 1;
        std::string key = lua_tostring(L, keyIndex);

        bool indexFound = false;
        if (key == "entityId") {
            auto* binding = (EntityBinding*) lua_touserdata(L, userdataIndex);
            lua_pushstring(L, binding->entityId.c_str());
            indexFound = true;
        }
        if (key == "type") {
            auto* binding = (EntityBinding*) lua_touserdata(L, userdataIndex);
            auto* scriptComponent = (ScriptComponent*) binding->getComponent("ScriptComponent");
            lua_pushstring(L, scriptComponent->Type.c_str());
            indexFound = true;
        }
        if (key == "transform") {
            auto* binding = (EntityBinding*) lua_touserdata(L, userdataIndex);
            auto* transformComponent = (TransformComponent*) binding->getComponent("TransformComponent");
            lua_newtable(L);
            lua_pushnumber(L, transformComponent->X);
            lua_setfield(L, -2, "x");
            lua_pushnumber(L, transformComponent->Y);
            lua_setfield(L, -2, "y");
            lua_pushnumber(L, transformComponent->Z);
            lua_setfield(L, -2, "z");
            indexFound = true;
        }
        if (key == "hasComponent") {
            auto function = [](lua_State* L) -> int {
                std::string componentType = lua_tostring(L, -1);
                auto* binding = (EntityBinding*) lua_touserdata(L, lua_upvalueindex(1));
                lua_pushboolean(L, binding->hasComponent(componentType));
                return 1;
            };
            auto* binding = (EntityBinding*) lua_touserdata(L, userdataIndex);
            lua_pushlightuserdata(L, binding);
            constexpr int upvalueCount = 1;
            lua_pushcclosure(L, function, upvalueCount);
            indexFound = true;
        }
        if (!indexFound) {
            // Key does not exist on C++ class, attempt to get it from lua table
            lua_getuservalue(L, userdataIndex);
            lua_pushvalue(L, keyIndex);
            lua_gettable(L, -2);
        }
        return 1;
    }

    static int destroy(lua_State* L) {
        auto* binding = (EntityBinding*) lua_touserdata(L, -1);
        binding->~EntityBinding();
        return 0;
    }

    static int create(lua_State* L) {
        std::string entityId = lua_tostring(L, 1);

        std::string typeName = lua_tostring(L, lua_upvalueindex(1));
        auto* scene = (Scene*) lua_touserdata(L, lua_upvalueindex(2));

        Entity& entity = scene->Entities[entityId];
        void* userdata = lua_newuserdata(L, sizeof(EntityBinding));
        new(userdata) EntityBinding(entityId, &entity);
        int userdataIndex = lua_gettop(L);

        std::string metatableName = typeName + "__metatable";
        luaL_getmetatable(L, metatableName.c_str());
        lua_setmetatable(L, userdataIndex);

        lua_newtable(L);
        lua_setuservalue(L, userdataIndex);

        lua_getglobal(L, typeName.c_str());
        const char* onCreateFn = "onCreate";
        lua_getfield(L, -1, onCreateFn);
        lua_setfield(L, -3, onCreateFn);
        const char* onUpdateFn = "onUpdate";
        lua_getfield(L, -1, onUpdateFn);
        lua_setfield(L, -3, onUpdateFn);
        const char* onDestroyFn = "onDestroy";
        lua_getfield(L, -1, onDestroyFn);
        lua_setfield(L, -3, onDestroyFn);
        lua_pop(L, 1);

        return 1;
    }
};

class Input {
public:
    static int create(lua_State* L) {
        println("Input create");
        return 0;
    }

    static int destroy(lua_State* L) {
        println("Input destroy");
        return 0;
    }

    static int index(lua_State* L) {
        println("Input index");
        return 0;
    }

    static int newIndex(lua_State* L) {
        println("Input newIndex");
        return 0;
    }
};

class Bar {
public:
    static void isKeyPressed() {
        println("TruE");
    }

    void isKeyPrezzed(const std::string& key) {
        println("KEY [%s] IS PRESSED", key.c_str());
    }
};

class Foo {
public:
    static void createType(lua_State* L, Bar* bar) {
        println("Foo create type");
        std::string typeName = "Foo";
        std::string metatableName = typeName + "__metatable";

        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, typeName.c_str());

        lua_pushlightuserdata(L, bar);
        int upvalueCount = 1;
        lua_pushcclosure(L, Foo::index, upvalueCount);
        lua_setfield(L, -2, "isKeyPressed");

        /*
        luaL_newmetatable(L, metatableName.c_str());
        {
            lua_pushstring(L, "__index");
            constexpr int upvalueCount = 0;
            lua_pushcclosure(L, Foo::index, upvalueCount);
            lua_settable(L, -3);
        }
        lua_setmetatable(L, -2);
        */
    }

    static int index(lua_State* L) {
        println("Foo index");
        printLua(L);
        std::string key = lua_tostring(L, -1);
        println("Foo index %s", key.c_str());
        auto* bar = (Bar*) lua_touserdata(L, lua_upvalueindex(1));
        bar->isKeyPrezzed(key);
        return 0;
    }
};

template<typename T, typename ...Arg>
void initLuaBindings(lua_State* L, const std::string& typeName, const Arg&... args) {
    std::string metatableName = typeName + "__metatable";

    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, typeName.c_str());
    {
        lua_pushstring(L, typeName.c_str());

        // https://www.fluentcpp.com/2021/03/12/cpp-fold-expressions/
        ([L, &args]() {
            lua_pushlightuserdata(L, (void*) &args);
        }(), ...);

        constexpr int upvalueCount = 1 + sizeof...(Arg);
        lua_pushcclosure(L, T::create, upvalueCount);
        lua_setfield(L, -2, "new");
    }

    luaL_newmetatable(L, metatableName.c_str());
    {
        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, T::destroy);
        lua_settable(L, -3);
    }
    {
        lua_pushstring(L, "__index");
        constexpr int upvalueCount = 0;
        lua_pushcclosure(L, T::index, upvalueCount);
        lua_settable(L, -3);
    }
    {
        lua_pushstring(L, "__newindex");
        constexpr int upvalueCount = 0;
        lua_pushcclosure(L, T::newIndex, upvalueCount);
        lua_settable(L, -3);
    }
}

int main() {
    println("Hello World");
    println("===");

    /*
     * Entities init
     */

    Entity player;
    player.Id = "player";
    player.ScriptComponent.Type = "Player";

    Entity camera;
    camera.Id = "camera";
    camera.ScriptComponent.Type = "Camera";

    Entity enemy;
    enemy.Id = "enemy";
    enemy.ScriptComponent.Type = "Player";

    Scene scene{};
    scene.Entities[player.Id] = player;
    scene.Entities[camera.Id] = camera;
    scene.Entities[enemy.Id] = enemy;

    for (const auto& iterator : scene.Entities) {
        const Entity& entity = iterator.second;
        scene.Types.insert(entity.ScriptComponent.Type);
    }

    /*
     * Lua init
     */

    lua_State* L = luaL_newstate();
    println("Created Lua state");

    luaL_openlibs(L);
    println("Opened Lua standard libraries");

    for (const std::string& typeName : scene.Types) {
        initLuaBindings<EntityBinding>(L, typeName, scene);
    }
    initLuaBindings<Input>(L, "Input", scene);

    auto* bar = new Bar();
    Foo::createType(L, bar);


    println("Registered Lua bindings");

    luaL_loadfile(L, "scripts");
    constexpr int argumentCount = 0;
    constexpr int resultCount = LUA_MULTRET;
    constexpr int messageHandlerIndex = 0;
    if (lua_pcall(L, argumentCount, resultCount, messageHandlerIndex) != LUA_OK) {
        std::stringstream ss;
        ss << "\n";
        ss << " Could not run lua scripts (\n";
        ss << "  " << lua_tostring(L, -1) << "\n";
        ss << " )\n";
        std::string error = ss.str();
        luaL_error(L, error.c_str());
    }
    println("Loaded Lua scripts");
    println("===");

    /*
     * Execution
     */

    lua_getglobal(L, "print");
    lua_pushstring(L, "Hello World from Lua from C++");
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        luaL_error(L, lua_tostring(L, -1));
    }

    for (const auto& iterator : scene.Entities) {
        const Entity& entity = iterator.second;

        lua_getglobal(L, entity.ScriptComponent.Type.c_str());
        println("hello? %s", entity.ScriptComponent.Type.c_str());
        lua_getfield(L, -1, "new");
        lua_pushstring(L, entity.Id.c_str());
        if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
            luaL_error(L, lua_tostring(L, -1));
        }

        lua_getglobal(L, "onCreateEntity");
        lua_pushvalue(L, -2);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            luaL_error(L, lua_tostring(L, -1));
        }
    }

    for (const auto& iterator : scene.Entities) {
        const Entity& entity = iterator.second;

        lua_getglobal(L, "onUpdateEntity");
        lua_pushstring(L, entity.Id.c_str());
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            luaL_error(L, lua_tostring(L, -1));
        }
    }

    lua_getglobal(L, "onDestroyEntity");
    lua_pushstring(L, scene.Entities["player"].Id.c_str());
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        luaL_error(L, lua_tostring(L, -1));
    }
    scene.Entities.erase("player");

    for (const auto& iterator : scene.Entities) {
        const Entity& entity = iterator.second;

        lua_getglobal(L, "onUpdateEntity");
        lua_pushstring(L, entity.Id.c_str());
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            luaL_error(L, lua_tostring(L, -1));
        }
    }

    /*
     * Cleanup
     */

    lua_close(L);
    return 0;
}