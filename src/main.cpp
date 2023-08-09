#include <lua.hpp>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <any>

#if 1
#define println(x, ...) printf(x, ##__VA_ARGS__); printf("\n")
#else
#define println(x, ...)
#endif

extern void printLua(lua_State* L, const std::string& tag = "LUA STACK");

extern void invokeLuaFunction(lua_State* L, const char* functionName, const char* signature, ...);

std::string getMetatableName(const std::string& tableName) {
    return tableName + "__meta";
}

struct ScriptComponent {
    std::string type;
};

struct TransformComponent {
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;

    TransformComponent() = default;

    TransformComponent(const TransformComponent&) = default;
};

struct Entity {
    std::string id;
    ScriptComponent scriptComponent;
    TransformComponent transformComponent;
};

struct Scene {
    std::unordered_map<std::string, Entity> entities;
    std::set<std::string> types;
};

struct Keyboard {
    static bool isKeyPressed(const std::string& key) {
        if (key == "Esc") {
            return true;
        }
        return false;
    }
};

struct EntityBinding {
private:
    Entity* entity;

public:
    explicit EntityBinding(Entity* entity) : entity(entity) {
        println("Created binding for entity with id [%s]", entity->id.c_str());
    }

    ~EntityBinding() {
        println("Destroyed binding for entity with id [%s]", entity->id.c_str());
    }

    std::string getId() const {
        return entity->id;
    }

    void setId(const std::string& id) const {
        entity->id = id;
    }

    void* getComponent(const std::string& componentType) const {
        if (componentType == "scriptComponent") {
            return &entity->scriptComponent;
        }
        if (componentType == "transformComponent") {
            return &entity->transformComponent;
        }
        return nullptr;
    }

    bool hasComponent(const std::string& componentType) const {
        if (getComponent(componentType) != nullptr) {
            return true;
        }
        return false;
    }

    static void createType(lua_State* L, const std::string& typeName, const Scene& scene) {
        std::string metatableName = getMetatableName(typeName);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, typeName.c_str());
        {
            lua_pushstring(L, typeName.c_str());
            lua_pushlightuserdata(L, (void*) &scene);
            constexpr int upvalueCount = 2;
            lua_pushcclosure(L, EntityBinding::createInstance, upvalueCount);
            lua_setfield(L, -2, "new");
        }
        luaL_newmetatable(L, metatableName.c_str());
        {
            lua_pushstring(L, "__gc");
            lua_pushcfunction(L, EntityBinding::destroy);
            lua_settable(L, -3);
        }
        {
            lua_pushstring(L, "__index");
            constexpr int upvalueCount = 0;
            lua_pushcclosure(L, EntityBinding::index, upvalueCount);
            lua_settable(L, -3);
        }
        {
            lua_pushstring(L, "__newindex");
            constexpr int upvalueCount = 0;
            lua_pushcclosure(L, EntityBinding::newIndex, upvalueCount);
            lua_settable(L, -3);
        }
    }

    static int createInstance(lua_State* L) {
        std::string typeName = lua_tostring(L, lua_upvalueindex(1));
        std::string metatableName = getMetatableName(typeName);

        std::string entityId = lua_tostring(L, 1);
        auto* scene = (Scene*) lua_touserdata(L, lua_upvalueindex(2));
        Entity& entity = scene->entities[entityId];

        void* userdata = lua_newuserdata(L, sizeof(EntityBinding));
        new(userdata) EntityBinding(&entity);
        int userdataIndex = lua_gettop(L);

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

    static int destroy(lua_State* L) {
        auto* binding = (EntityBinding*) lua_touserdata(L, -1);
        binding->~EntityBinding();
        return 0;
    }

    static int index(lua_State* L) {
        constexpr int bottomOfLuaStackIndex = 1;
        int userdataIndex = bottomOfLuaStackIndex;
        int keyIndex = userdataIndex + 1;
        std::string key = lua_tostring(L, keyIndex);

        bool indexFound = false;
        if (key == "entityId") {
            auto* binding = (EntityBinding*) lua_touserdata(L, userdataIndex);
            lua_pushstring(L, binding->getId().c_str());
            indexFound = true;
        }
        if (key == "type") {
            auto* binding = (EntityBinding*) lua_touserdata(L, userdataIndex);
            auto* scriptComponent = (ScriptComponent*) binding->getComponent("scriptComponent");
            lua_pushstring(L, scriptComponent->type.c_str());
            indexFound = true;
        }
        if (key == "transform") {
            auto* binding = (EntityBinding*) lua_touserdata(L, userdataIndex);
            auto* transformComponent = (TransformComponent*) binding->getComponent("transformComponent");
            lua_newtable(L);
            lua_pushnumber(L, transformComponent->x);
            lua_setfield(L, -2, "x");
            lua_pushnumber(L, transformComponent->y);
            lua_setfield(L, -2, "y");
            lua_pushnumber(L, transformComponent->z);
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
            binding->setId(value);
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
};

struct KeyboardBinding {

    KeyboardBinding() {
        println("Created binding for mouse & keyboard keyboard");
    }

    ~KeyboardBinding() {
        println("Destroyed binding for mouse & keyboard keyboard");
    }

    static void createType(lua_State* L, const std::string& typeName) {
        std::string metatableName = getMetatableName(typeName);

        void* userdata = lua_newuserdata(L, sizeof(KeyboardBinding));
        new(userdata) KeyboardBinding();
        int userdataIndex = lua_gettop(L);

        luaL_newmetatable(L, metatableName.c_str());
        {
            lua_pushstring(L, "__gc");
            lua_pushcfunction(L, KeyboardBinding::destroy);
            lua_settable(L, -3);
        }
        {
            lua_pushstring(L, "__index");
            constexpr int upvalueCount = 0;
            lua_pushcclosure(L, KeyboardBinding::index, upvalueCount);
            lua_settable(L, -3);
        }
        lua_setmetatable(L, userdataIndex);

        lua_newtable(L);
        lua_setuservalue(L, userdataIndex);

        lua_pushvalue(L, -1);
        lua_setglobal(L, typeName.c_str());
    }

    static int destroy(lua_State* L) {
        auto* binding = (KeyboardBinding*) lua_touserdata(L, -1);
        binding->~KeyboardBinding();
        return 0;
    }

    static int index(lua_State* L) {
        constexpr int bottomOfLuaStackIndex = 1;
        int userdataIndex = bottomOfLuaStackIndex;

        int keyIndex = userdataIndex + 1;
        std::string key = lua_tostring(L, keyIndex);

        if (key == "isKeyPressed") {
            auto function = [](lua_State* L) -> int {
                std::string key = lua_tostring(L, -1);
                lua_pushboolean(L, Keyboard::isKeyPressed(key));
                return 1;
            };
            lua_pushcfunction(L, function);
            return 1;
        }

        return 0;
    }
};

struct LuaFunction {
    std::string name;
    std::string tableName;
    int argumentCount = 0;
    int returnValueCount = 0;
    int messageHandlerIndex = 0;
    std::function<void(lua_State*)> onPushArguments = nullptr;
    std::function<void(lua_State*)> onError = nullptr;

    void invoke(lua_State* L) const {
        if (!tableName.empty()) {
            lua_getglobal(L, tableName.c_str());
            lua_getfield(L, -1, name.c_str());
        } else if (!name.empty()) {
            lua_getglobal(L, name.c_str());
        }
        if (onPushArguments) {
            onPushArguments(L);
        }
        if (lua_pcall(L, argumentCount, returnValueCount, messageHandlerIndex) != LUA_OK) {
            if (onError) {
                onError(L);
            } else {
                luaL_error(L, "Could not invoke function [%s]: %s", name.c_str(), lua_tostring(L, -1));
            }
        }
    }
};

int main() {
    println("Hello World");
    println("===");

    /*
     * Domain init
     */

    Entity player;
    player.id = "player";
    player.scriptComponent.type = "Player";

    Entity camera;
    camera.id = "camera";
    camera.scriptComponent.type = "Camera";

    Scene scene{};
    scene.entities[player.id] = player;
    scene.entities[camera.id] = camera;

    for (const auto& iterator : scene.entities) {
        const Entity& entity = iterator.second;
        scene.types.insert(entity.scriptComponent.type);
    }

    /*
     * Lua init
     */

    lua_State* L = luaL_newstate();
    println("Created Lua state");

    luaL_openlibs(L);
    println("Opened Lua standard libraries");

    for (const std::string& typeName : scene.types) {
        EntityBinding::createType(L, typeName, scene);
    }
    KeyboardBinding::createType(L, "Keyboard");
    println("Registered Lua bindings");

    luaL_loadfile(L, "luac.out");
    {
        LuaFunction function{};
        function.argumentCount = 0;
        function.returnValueCount = LUA_MULTRET;
        function.onError = [](lua_State* L) {
            std::stringstream ss;
            ss << "\n";
            ss << " Could not run lua scripts (\n";
            ss << "  " << lua_tostring(L, -1) << "\n";
            ss << " )\n";
            std::string error = ss.str();
            luaL_error(L, error.c_str());
        };
        function.invoke(L);
    }
    println("Loaded Lua scripts");
    println("===");

    /*
     * Execution
     */

    {
        LuaFunction function{};
        function.name = "print";
        function.argumentCount = 1;
        function.onPushArguments = [](lua_State* L) {
            lua_pushstring(L, "Hello World from Lua from C++");
        };
        function.invoke(L);
    }

    for (const auto& iterator : scene.entities) {
        const Entity& entity = iterator.second;
        {
            LuaFunction function{};
            function.name = "new";
            function.tableName = entity.scriptComponent.type;
            function.argumentCount = 1;
            function.returnValueCount = 1;
            function.onPushArguments = [&entity](lua_State* L) {
                lua_pushstring(L, entity.id.c_str());
            };
            function.invoke(L);
        }
        {
            LuaFunction function{};
            function.name = "onCreateEntity";
            function.argumentCount = 1;
            function.onPushArguments = [](lua_State* L) {
                lua_pushvalue(L, -2);
            };
            function.invoke(L);
        }
    }

    for (const auto& iterator : scene.entities) {
        const Entity& entity = iterator.second;
        {
            LuaFunction function{};
            function.name = "onUpdateEntity";
            function.argumentCount = 1;
            function.onPushArguments = [&entity](lua_State* L) {
                lua_pushstring(L, entity.id.c_str());
            };
            function.invoke(L);
        }
    }

    {
        LuaFunction function{};
        function.name = "onDestroyEntity";
        function.argumentCount = 1;
        function.onPushArguments = [&player](lua_State* L) {
            lua_pushstring(L, player.id.c_str());
        };
        function.invoke(L);
    }
    scene.entities.erase(player.id);

    for (const auto& iterator : scene.entities) {
        const Entity& entity = iterator.second;
        {
            LuaFunction function{};
            function.name = "onUpdateEntity";
            function.argumentCount = 1;
            function.onPushArguments = [&entity](lua_State* L) {
                lua_pushstring(L, entity.id.c_str());
            };
            function.invoke(L);
        }
    }

    /*
     * Cleanup
     */

    lua_close(L);
    return 0;
}
