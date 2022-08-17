#include <lua.hpp>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

extern void printLua(lua_State* L, const std::string& tag = "NOTICE ME");

struct ScriptComponent {
    std::string Type;
};

struct Entity {
    std::string Id;
    ScriptComponent ScriptComponent;
};

struct Scene {
    std::unordered_map<std::string, Entity> Entities;
    std::set<std::string> Types;
};

struct EntityBinding {
    std::string entityId = "FOO_ENTITY";

    EntityBinding() {
        printf("FOO CREATED\n");
    }

    explicit EntityBinding(const std::string& entityId) : entityId(entityId) {
        printf("FOO CREATED with entity id [%s]\n", entityId.c_str());
    }

    ~EntityBinding() {
        printf("FOO DESTROYED\n");
    }
};

static int newIndex(lua_State* L) {
    printf("NEW_INDEX\n");

    constexpr int bottomOfLuaStackIndex = 1;
    int userdataIndex = bottomOfLuaStackIndex;
    int keyIndex = userdataIndex + 1;
    int valueIndex = keyIndex + 1;

    const char* key = lua_tostring(L, keyIndex);

    bool indexFound = false;
    if (strcmp(key, "entityId") == 0) {
        const char* value = lua_tostring(L, valueIndex);
        auto* foo = (EntityBinding*) lua_touserdata(L, userdataIndex);
        foo->entityId = std::string(value);
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

static int destroy(lua_State* L) {
    auto* foo = (EntityBinding*) lua_touserdata(L, -1);
    foo->~EntityBinding();
    return 0;
}

static int index(lua_State* L) {
    printf("INDEX\n");

    constexpr int bottomOfLuaStackIndex = 1;
    int userdataIndex = bottomOfLuaStackIndex;
    int keyIndex = userdataIndex + 1;

    const char* key = lua_tostring(L, keyIndex);
    printf("INdexing %s\n", key);

    bool indexFound = false;
    if (strcmp(key, "entityId") == 0) {
        auto* foo = (EntityBinding*) lua_touserdata(L, userdataIndex);
        lua_pushstring(L, foo->entityId.c_str());
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

static int create(lua_State* L) {
    printf("CREATE\n");

    // MAKE ME DYNAMIC ??
    std::string typeName = "Player";
    std::string metatableName = typeName + "__metatable";

    const char* eid = lua_tostring(L, 1);
    printf("EID %s\n", eid);

    void* userdata = lua_newuserdata(L, sizeof(EntityBinding));
    new(userdata) EntityBinding(eid);
    int userdataIndex = lua_gettop(L);

    luaL_getmetatable(L, metatableName.c_str());
    lua_setmetatable(L, userdataIndex);

    lua_newtable(L);
    lua_setuservalue(L, userdataIndex);

    // instead of returning userdata table,
    // return a new table with onCreate+onUpdate fields from type, and with userdata as metatable

    lua_getglobal(L, typeName.c_str());
    if (!lua_isnil(L, -1)) {
        lua_getfield(L, -1, "onCreate");
        lua_setfield(L, -3, "onCreate");
        lua_getfield(L, -1, "onUpdate");
        lua_setfield(L, -3, "onUpdate");
    }
    lua_pop(L, 1);

    /*
    const char* type = lua_tostring(L, lua_upvalueindex(1));
    printf("I am of type [%s]\n", type);

    auto* scene = (Scene*) lua_touserdata(L, lua_upvalueindex(2));
    printf("I have [%d] entities\n", (int) scene->Entities.size());
    */

    return 1;
}

void initLuaBindings(lua_State* L, Scene* scene) {
    for (const std::string& typeName : scene->Types) {

        std::string metatableName = typeName + "__metatable";

        printf("INIT -- [%s], [%s]\n", typeName.c_str(), metatableName.c_str());

        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, typeName.c_str());
        {
            lua_pushlightuserdata(L, (void*) scene);
            constexpr int upvalueCount = 1;
            lua_pushcclosure(L, create, upvalueCount);
            lua_setfield(L, -2, "new");
        }

        luaL_newmetatable(L, metatableName.c_str());
        {
            lua_pushstring(L, "__gc");
            lua_pushcfunction(L, destroy);
            lua_settable(L, -3);
        }
        {
            lua_pushstring(L, "__index");
            lua_pushlightuserdata(L, (void*) scene);
            constexpr int upvalueCount = 1;
            lua_pushcclosure(L, index, upvalueCount);
            lua_settable(L, -3);
        }
        {
            lua_pushstring(L, "__newindex");
            lua_pushlightuserdata(L, (void*) scene);
            constexpr int upvalueCount = 1;
            lua_pushcclosure(L, newIndex, upvalueCount);
            lua_settable(L, -3);
        }
    }
}

int main() {
    printf("Hello World\n");
    printf("===\n");

    /*
     * Scene/Entities init
     */

    Scene scene{};
    scene.Entities["player"] = {"player", {"Player"}};
    scene.Entities["foo"] = {"foo", {"Player"}};
    scene.Entities["bar"] = {"bar", {"Player"}};
    for (const auto& iterator : scene.Entities) {
        const Entity& entity = iterator.second;
        scene.Types.insert(entity.ScriptComponent.Type);
    }
    /*
     * Lua init
     */

    lua_State* L = luaL_newstate();
    printf("Created Lua state\n");

    luaL_openlibs(L);
    printf("Opened Lua standard libraries\n");

    initLuaBindings(L, &scene);
    printf("Registered userdatums in Lua\n");

    printf("===\n");
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
    printf("===\n");
    printf("Loaded Lua scripts\n");
    printf("===\n");

    /*
     * Program execution
     */

    lua_getglobal(L, "print");
    lua_pushstring(L, "Hello World from Lua from C++");
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        luaL_error(L, lua_tostring(L, -1));
    }

    lua_getglobal(L, "Player");
    lua_getfield(L, -1, "new");
    lua_pushstring(L, "foobar123456789");
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        luaL_error(L, lua_tostring(L, -1));
    }

    lua_getfield(L, -1, "onUpdate");
    lua_pushvalue(L, -2);
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        luaL_error(L, lua_tostring(L, -1));
    }

    lua_getglobal(L, "Player");
    lua_getfield(L, -1, "new");
    lua_pushstring(L, "some_completely_other_id_because_reasons");
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        luaL_error(L, lua_tostring(L, -1));
    }

    lua_getfield(L, -1, "onUpdate");
    lua_pushvalue(L, -2);
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        luaL_error(L, lua_tostring(L, -1));
    }

    return 0;
}