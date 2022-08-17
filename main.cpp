#include <lua.hpp>
#include <iostream>
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
};

struct EntityBinding {
    std::string entityId = "FOO_ENTITY";

    EntityBinding() {
        printf("FOO CREATED\n");
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
        // Set new index on lua table (i.e. index that does not exist on C++ class)
        lua_getuservalue(L, userdataIndex);
        lua_pushvalue(L, keyIndex);
        lua_pushvalue(L, valueIndex);
        lua_settable(L, -3);
    }
    return 1;
}

static int destroy(lua_State* L) {
    printf("DESTROY\n");
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

    bool indexFound = false;
    if (strcmp(key, "entityId") == 0) {
        auto* foo = (EntityBinding*) lua_touserdata(L, userdataIndex);
        lua_pushstring(L, foo->entityId.c_str());
        indexFound = true;
    }
    if (!indexFound) {
        // Attempt to get value from lua table
        lua_getuservalue(L, userdataIndex);
        lua_pushvalue(L, keyIndex);
        lua_gettable(L, -2);
    }
    return 1;
}

static int createInstance(lua_State* L) {
    printf("CREATE\n");

    std::string typeName = "Entity";
    std::string metatableName = typeName + "__metatable";

    void* userdata = lua_newuserdata(L, sizeof(EntityBinding));
    new(userdata) EntityBinding();
    int userdataIndex = lua_gettop(L);

    luaL_getmetatable(L, metatableName.c_str());
    lua_setmetatable(L, userdataIndex);

    lua_newtable(L);
    lua_setuservalue(L, userdataIndex);

    constexpr int createdCount = 1;
    return createdCount;
}

static int createType(lua_State* L) {
    printf("CREATE_TYPE\n");

    std::string typeName = "Entity";

    // o
    lua_newtable(L);

    // self
    lua_getglobal(L, typeName.c_str());

    // setmetatable(o, self)
    lua_setmetatable(L, -2);

    // self.__index = self
    lua_getglobal(L, typeName.c_str());
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    lua_pop(L, 1);

    constexpr int upvalueCount = 0;
    lua_pushcclosure(L, createInstance, upvalueCount);
    lua_setfield(L, -2, "new");

    return 1;
}

void foo(lua_State* L, Scene* scene) {
    std::string typeName = "Entity";
    std::string metatableName = typeName + "__metatable";

    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, typeName.c_str());
    {
        lua_pushlightuserdata(L, (void*) scene);
        constexpr int upvalueCount = 1;
        lua_pushcclosure(L, createType, upvalueCount);
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

    /*
     * Lua init
     */

    lua_State* L = luaL_newstate();
    printf("Created Lua state\n");

    luaL_openlibs(L);
    printf("Opened Lua standard libraries\n");

    foo(L, &scene);
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

    return 0;
}

/*
         * function Entity:new(entity)
         *   entity = entity or {}
         *   setmetatable(entity, self)
         *   self.__index = self
         *   return entity
         * end
         */

/*

 // self
    lua_newtable(L);
    lua_setuservalue(L, userdataIndex);

    // o
    lua_newtable(L);

    // setmetatable(o, self)
    lua_pushvalue(L, -2);
    lua_setmetatable(L, -2);

    // self.__index = self
    lua_pushvalue(L, -2);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    lua_pop(L, 1);

*/



/*
printLua(L, "createFn");

// entity = entity or {}
if (lua_gettop(L) == 0) {
lua_newtable(L);
} else {

}

// setmetatable(entity, self)

lua_getglobal(L, "Entity");
lua_setmetatable(L, -2);

// self.__index = self
//printLua(L, "one");
lua_getglobal(L, "Entity");
//printLua(L, "two");
lua_pushvalue(L, -1);
//printLua(L, "three");
lua_pushstring(L, "__index");
//printLua(L, "four");
//lua_settable(L, -3);
*/