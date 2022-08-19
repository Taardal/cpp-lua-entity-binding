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

struct Input {
    bool isKeyPressed(const std::string& key) const {
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
        if (componentType == "scriptComponent") {
            return true;
        }
        if (componentType == "transformComponent") {
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

class InputBinding {
private:
    const Input* input;

public:
    explicit InputBinding(const Input* input) : input(input) {
        println("Created binding for mouse & keyboard input");
    }

    ~InputBinding() {
        println("Destroyed binding for mouse & keyboard input");
    }

    bool isKeyPressed(const std::string& key) const {
        return input->isKeyPressed(key);
    }

    static void createType(lua_State* L, const std::string& typeName, const Input& input) {
        std::string metatableName = getMetatableName(typeName);

        void* userdata = lua_newuserdata(L, sizeof(InputBinding));
        new(userdata) InputBinding(&input);
        int userdataIndex = lua_gettop(L);

        luaL_newmetatable(L, metatableName.c_str());
        {
            lua_pushstring(L, "__gc");
            lua_pushcfunction(L, InputBinding::destroy);
            lua_settable(L, -3);
        }
        {
            lua_pushstring(L, "__index");
            constexpr int upvalueCount = 0;
            lua_pushcclosure(L, InputBinding::index, upvalueCount);
            lua_settable(L, -3);
        }
        lua_setmetatable(L, userdataIndex);

        lua_newtable(L);
        lua_setuservalue(L, userdataIndex);

        lua_pushvalue(L, -1);
        lua_setglobal(L, typeName.c_str());
    }

    static int destroy(lua_State* L) {
        auto* binding = (InputBinding*) lua_touserdata(L, -1);
        binding->~InputBinding();
        return 0;
    }

    static int index(lua_State* L) {
        constexpr int bottomOfLuaStackIndex = 1;
        int userdataIndex = bottomOfLuaStackIndex;
        auto* binding = (InputBinding*) lua_touserdata(L, userdataIndex);

        int keyIndex = userdataIndex + 1;
        std::string key = lua_tostring(L, keyIndex);

        if (key == "isKeyPressed") {
            auto function = [](lua_State* L) -> int {
                std::string key = lua_tostring(L, -1);
                auto* binding = (InputBinding*) lua_touserdata(L, lua_upvalueindex(1));
                lua_pushboolean(L, binding->isKeyPressed(key));
                return 1;
            };
            lua_pushlightuserdata(L, binding);
            constexpr int upvalueCount = 1;
            lua_pushcclosure(L, function, upvalueCount);
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

    template<typename ...Argument>
    void invoke(lua_State* L, const char* signature, const Argument& ...argument) {
        if (!tableName.empty()) {
            lua_getglobal(L, tableName.c_str());
            lua_getfield(L, -1, name.c_str());
        } else if (!name.empty()) {
            lua_getglobal(L, name.c_str());
        }
        LuaFunction::invoke(L, signature, argument...);
    }
    
    template<typename ...Argument>
    static void invokeOnTable(lua_State* L, const char* tableName, const char* functionName, const char* signature, const Argument& ...argument) {
        lua_getglobal(L, tableName);
        lua_getfield(L, -1, functionName);
        if (invoke(L, signature, argument...) != LUA_OK) {
            luaL_error(L, "Could not invoke function [%s] on table [%s]: %s", functionName, tableName, lua_tostring(L, -1));
        }
    }

    template<typename ...Argument>
    static void invoke(lua_State* L, const char* functionName, const char* signature, const Argument& ...argument) {
        lua_getglobal(L, functionName);
        if (invokeWithSignature(L, signature, argument...) != LUA_OK) {
            luaL_error(L, "Could not invoke function [%s]: %s", functionName, lua_tostring(L, -1));
        }
    }

private:
    static int invokeWithSignature(lua_State* L, const char* signature, ...) {
        va_list argumentList;
        va_start(argumentList, signature);

        int argumentCount = 0;
        bool allArgumentsPushed = false;
        while (*signature && !allArgumentsPushed) {
            switch (*signature++) {
                case 'd':
                    lua_pushnumber(L, va_arg(argumentList, double));
                    break;
                case 'i':
                    lua_pushnumber(L, va_arg(argumentList, int));
                    break;
                case 's':
                    lua_pushstring(L, va_arg(argumentList, char*));
                    break;
                case '>':
                    allArgumentsPushed = true;
                    break;
                default:
                    luaL_error(L, "Invalid argument type specified (%c)", *(signature - 1));
            }
            argumentCount++;
            constexpr int minimumRequiredStackSpace = 1;
            luaL_checkstack(L, minimumRequiredStackSpace, "Too many arguments on the stack");
        }

        int returnValueCount = (int) strlen(signature);
        constexpr int messageHandlerIndex = 0;
        int result = lua_pcall(L, argumentCount, returnValueCount, messageHandlerIndex);
        if (result != LUA_OK) {
            return result;
        }

        returnValueCount = -returnValueCount; // stack index of first result
        while (*signature) {
            switch (*signature++) {
                case 'd':
                    if (!lua_isnumber(L, returnValueCount)) {
                        luaL_error(L, "wrong result type");
                    }
                    *va_arg(argumentList, double*) = lua_tonumber(L, returnValueCount);
                    break;
                case 'i':
                    if (!lua_isnumber(L, returnValueCount)) {
                        luaL_error(L, "wrong result type");
                    }
                    *va_arg(argumentList, int*) = (int)lua_tonumber(L, returnValueCount);
                    break;
                case 's':
                    if (!lua_isstring(L, returnValueCount)) {
                        luaL_error(L, "wrong result type");
                    }
                    *va_arg(argumentList, const char**) = lua_tostring(L, returnValueCount);
                    break;
                default:
                    luaL_error(L, "Invalid return type specified (%c)", *(signature - 1));
            }
            returnValueCount++;
        }
        va_end(argumentList);
        return LUA_OK;
    }
};

/**
 * Lua: A Generic Call Function
 *
 * This functions does the following:
 *   1. Pushes the function to the stack
 *   2. Pushes the arguments to the stack
 *   3. Invokes the function
 *   4. Gets the results
 *
 * The signature are letters which specifies arguments and results
 *   - `d´ for double
 *   - `i´ for integer
 *   - `s´ for strings
 *   - `>´ separates arguments from the results. If the function has no results, the `>´ is optional.
 *   Example:
 *     - "dd>s" means "two arguments of type double, one result of type double"
 *     - "ds" means "one argument of type double, one argument of type string, no results"
 *
 * Things to note:
 *   1. It does not need to check whether func is a function; lua_pcall will trigger any occasional error.
 *   2. Because it pushes an arbitrary number of arguments, it must check the stack space.
 *   3. Because the function may return strings, it cannot pop the results from the stack. It is up to the caller to pop them.
 *
 * Source:
 *   - https://www.lua.org/pil/25.3.html
 */
void invokeLuaFn(lua_State* L, const char* functionName, const char* signature, ...) {
    lua_getglobal(L, functionName);

    va_list argumentList;
    va_start(argumentList, signature);

    int argumentCount = 0;
    bool allArgumentsPushed = false;
    while (*signature && !allArgumentsPushed) {
        switch (*signature++) {
            case 'd':
                lua_pushnumber(L, va_arg(argumentList, double));
                break;
            case 'i':
                lua_pushnumber(L, va_arg(argumentList, int));
                break;
            case 's':
                lua_pushstring(L, va_arg(argumentList, char*));
                break;
            case '>':
                allArgumentsPushed = true;
                break;
            default:
                luaL_error(L, "Invalid argument type specified (%c)", *(signature - 1));
        }
        argumentCount++;
        constexpr int minimumRequiredStackSpace = 1;
        luaL_checkstack(L, minimumRequiredStackSpace, "Too many arguments on the stack");
    }

    int returnValueCount = (int) strlen(signature);
    constexpr int messageHandlerIndex = 0;
    if (lua_pcall(L, argumentCount, returnValueCount, messageHandlerIndex) != LUA_OK) {
        luaL_error(L, "Could not invoke function [%s]: %s", functionName, lua_tostring(L, -1));
    }

    returnValueCount = -returnValueCount; // stack index of first result
    while (*signature) {
        switch (*signature++) {
            case 'd':
                if (!lua_isnumber(L, returnValueCount)) {
                    luaL_error(L, "wrong result type");
                }
                *va_arg(argumentList, double*) = lua_tonumber(L, returnValueCount);
                break;
            case 'i':
                if (!lua_isnumber(L, returnValueCount)) {
                    luaL_error(L, "wrong result type");
                }
                *va_arg(argumentList, int*) = (int)lua_tonumber(L, returnValueCount);
                break;
            case 's':
                if (!lua_isstring(L, returnValueCount)) {
                    luaL_error(L, "wrong result type");
                }
                *va_arg(argumentList, const char**) = lua_tostring(L, returnValueCount);
                break;
            default:
                luaL_error(L, "Invalid return type specified (%c)", *(signature - 1));
        }
        returnValueCount++;
    }
    va_end(argumentList);
}

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

    Input input{};

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
    InputBinding::createType(L, "Input", input);
    println("Registered Lua bindings");

    luaL_loadfile(L, "scripts");
    {
        LuaFunction fn{};
        fn.argumentCount = 0;
        fn.returnValueCount = LUA_MULTRET;
        fn.onError = [](lua_State* L) {
            std::stringstream ss;
            ss << "\n";
            ss << " Could not run lua scripts (\n";
            ss << "  " << lua_tostring(L, -1) << "\n";
            ss << " )\n";
            std::string error = ss.str();
            luaL_error(L, error.c_str());
        };
        fn.invoke(L);
    }
    println("Loaded Lua scripts");
    println("===");

    /*
     * Execution
     */

    //LuaFunction::invoke(L, "print", "s", "Hello World from Lua from C++");
    {
        LuaFunction fn{};
        fn.name = "print";
        fn.argumentCount = 1;
        fn.onPushArguments = [](lua_State* L) {
            lua_pushstring(L, "Hello World from Lua from C++");
        };
        fn.invoke(L);
    }


    for (const auto& iterator : scene.entities) {
        const Entity& entity = iterator.second;
        {
            LuaFunction fn{};
            fn.name = "new";
            fn.tableName = entity.scriptComponent.type;
            fn.argumentCount = 1;
            fn.returnValueCount = 1;
            fn.onPushArguments = [&entity](lua_State* L) {
                lua_pushstring(L, entity.id.c_str());
            };
            fn.invoke(L);
        }
        {
            LuaFunction fn{};
            fn.name = "onCreateEntity";
            fn.argumentCount = 1;
            fn.onPushArguments = [](lua_State* L) {
                lua_pushvalue(L, -2);
            };
            fn.invoke(L);
        }
    }

    for (const auto& iterator : scene.entities) {
        const Entity& entity = iterator.second;
        //LuaFunction::invoke(L, "onUpdateEntity", "s", entity.id.c_str());
        {
            LuaFunction fn{};
            fn.name = "onUpdateEntity";
            fn.argumentCount = 1;
            fn.onPushArguments = [&entity](lua_State* L) {
                lua_pushstring(L, entity.id.c_str());
            };
            fn.invoke(L);
        }
    }

    //LuaFunction::invoke(L, "onDestroyEntity", "s", player.id.c_str());
    {
        LuaFunction fn{};
        fn.name = "onDestroyEntity";
        fn.argumentCount = 1;
        fn.onPushArguments = [&player](lua_State* L) {
            lua_pushstring(L, player.id.c_str());
        };
        fn.invoke(L);
    }
    scene.entities.erase("player");

    for (const auto& iterator : scene.entities) {
        const Entity& entity = iterator.second;
        //LuaFunction::invoke(L, "onUpdateEntity", "s", entity.id.c_str());
        {
            LuaFunction fn{};
            fn.name = "onUpdateEntity";
            fn.argumentCount = 1;
            fn.onPushArguments = [&entity](lua_State* L) {
                lua_pushstring(L, entity.id.c_str());
            };
            fn.invoke(L);
        }
    }

    /*
     * Cleanup
     */

    lua_close(L);
    return 0;
}