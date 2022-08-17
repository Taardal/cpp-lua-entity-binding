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

static int newIndex(lua_State* L) {
    printf("NEW_INDEX\n");
    return 0;
}

static int index(lua_State* L) {
    printf("INDEX\n");
    return 0;
}

static int destroy(lua_State* L) {
    printf("DESTROY\n");
    return 0;
}

static int create(lua_State* L) {
    printf("CREATE\n");
    return 0;
}

void foo(lua_State* L, Scene* scene) {
    std::string typeName = "Entity";

    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, typeName.c_str());



    {
        lua_pushlightuserdata(L, (void*) scene);
        constexpr int upvalueCount = 1;
        lua_pushcclosure(L, create, upvalueCount);
        lua_setfield(L, -2, "new");
        //printf("added new/create function with upvalue [%s]\n", typeName.c_str());
    }

    std::string metatableName = typeName + "__metatable";
    luaL_newmetatable(L, metatableName.c_str());
    //printf("created metatable [%s]\n", metatableName.c_str());

    {
        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, destroy);
        lua_settable(L, -3);
        //printf("added garbage collect function to metatable [%s]\n", metatableName.c_str());
    }
    {
        lua_pushstring(L, "__index");
        lua_pushlightuserdata(L, (void*) scene);
        constexpr int upvalueCount = 1;
        lua_pushcclosure(L, index, upvalueCount);
        lua_settable(L, -3);
        //printf("added index function with upvalue [%s] to metatable [%s]\n", typeName.c_str(), metatableName.c_str());
    }
    {
        lua_pushstring(L, "__newindex");
        lua_pushlightuserdata(L, (void*) scene);
        constexpr int upvalueCount = 1;
        lua_pushcclosure(L, newIndex, upvalueCount);
        lua_settable(L, -3);
        //printf("added newindex function with upvalue [%s] to metatable [%s]\n", typeName.c_str(), metatableName.c_str());
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
    luaL_dofile(L, "scripts");
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