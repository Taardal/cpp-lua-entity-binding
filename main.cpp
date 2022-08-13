#include <lua.hpp>
#include <iostream>

struct LuaFunction {
  std::string Name = "";
  int ArgumentCount = 0;
  int ResultsCount = 0;
};

void invokeLuaFunction(lua_State* L, const LuaFunction& luaFunction) {
    lua_getglobal(L, luaFunction.Name.c_str());
    constexpr int messageHandlerIndex = 0;
    if (lua_pcall(L, luaFunction.ArgumentCount, luaFunction.ResultsCount, messageHandlerIndex) != LUA_OK) {
        luaL_error(L, "Could not call method [%s]: %s", luaFunction.Name.c_str(), lua_tostring(L, -1));
    }
}

static int log(lua_State* L) {
  const char* message = lua_tostring(L, -1);
  printf("[Lua] %s\n", message);
  return 0; 
}

int main() {
	printf("Hello World\n");

	lua_State* L = luaL_newstate();
	printf("Created Lua state\n");

  luaL_openlibs(L);
	printf("Opened Lua standard libraries\n");

  lua_register(L, "log", log);
  printf("Registered native functions in Lua\n");

  luaL_dofile(L, "scripts");
  printf("Loaded Lua scripts");

  const char* script = R"(
    log("Hello World from Lua")

    createEntity(Player.new { entityId = "player" })
    printEntities()


  )";
  
  luaL_dostring(L, script);
  
  

  /*
  std::vector scriptComponentTypes = {
      "Player"
  };
  */

  // Parse script component types to ScriptClass 
  //   add to map by type string (?)
  //
  // For each ScriptClass
  //   ScriptInstance instance = scriptClass.instantiate() --> new lua table + call onCreate() function
  //   add instance to map by entity id
  //   
  //  OnUpdate
  //    For each ScriptInstance
  //      instance.onUpdate()
  //
}
