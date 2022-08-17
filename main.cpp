#include <lua.hpp>
#include <iostream>
#include <sstream>
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
    std::vector<Entity> Entities;
};

int main() {
    printf("Hello World\n");
    printf("===\n");

    /*
     * Lua init
     */

    lua_State* L = luaL_newstate();
    printf("Created Lua state\n");

    luaL_openlibs(L);
    printf("Opened Lua standard libraries\n");

    /*
     * Scene/Entities init
     */

    Scene scene{};
    scene.Entities = {
            {"player", {"Player"}},
            {"foo",    {"Player"}},
            {"bar",    {"Player"}}
    };

    /*
     * Lua functions
     */

    lua_register(L, "printNative", [](lua_State* L) -> int {
        printf("[Lua] %s\n", lua_tostring(L, -1));
        return 0;
    });
    printf("Registered native functions in Lua\n");

    /*
     * Lua userdatums
     */

    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "Entity");

    lua_pushstring(L, "bar");
    lua_setfield(L, -2, "foo");

    lua_pushlightuserdata(L, (void*) &scene);
    constexpr int upvalueCount = 1;
    auto createFn = [](lua_State* L) -> int {

        /*
         * function Entity:new(entity)
         *   entity = entity or {}
         *   setmetatable(entity, self)
         *   self.__index = self
         *   return entity
         * end
         */

        printLua(L, "createFn");

        // entity = entity or {}
        if (lua_gettop(L) == 0) {
            lua_newtable(L);
        } else {
            /*
            lua_getfield(L, -1, "entityId");
            printf("%s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
            */

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


        return 1;
    };
    lua_pushcclosure(L, createFn, upvalueCount);
    lua_setfield(L, -2, "new");
    lua_pop(L, 1);


    /*
    luaL_newmetatable(L, "Entity");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    */

    printf("Registered userdatums in Lua\n");

    /*
     * Lua script loading
     */

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

    printLua(L, "mark");
    printf("%s\n", lua_tostring(L, -1));

    lua_getglobal(L, "Player");
    lua_getfield(L, -1, "new");

    //lua_pushlightuserdata(L, &scene);

    lua_newtable(L);
    lua_pushstring(L, "SOME ENTITY");
    lua_setfield(L, -2, "entityId");

    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        luaL_error(L, lua_tostring(L, -1));
    }
    printf("Created player instance\n");

    lua_getfield(L, -2, "foo");
    printf("%s\n", lua_tostring(L, -1));

    return 0;
    lua_getfield(L, -1, "onUpdate");
    lua_pushvalue(L, -2);
    //lua_pushstring(L, "bbbb");
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        luaL_error(L, lua_tostring(L, -1));
    }
    printf("Updated player instance\n");

    printf("Entities:\n");
    for (auto& entity : scene.Entities) {
        printf("  - %s (%s)\n", entity.Id.c_str(), entity.ScriptComponent.Type.c_str());
    }

    return 0;
}
