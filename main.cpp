#include <lua.hpp>
#include <iostream>
#include <sstream>
#include <vector>

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

static int printNative(lua_State* L) {
  const char* message = lua_tostring(L, -1);
  printf("[Lua] %s\n", message);
  return 0; 
}

struct ScriptComponent {
  std::string Type;
};

struct Entity {
  std::string Id;
  ScriptComponent ScriptComponent;
};

struct ScriptInstance {
  std::string EntityId;

  void onCreate(lua_State* L) {
    std::stringstream ss;
    ss << "initEntity('" << EntityId << "')";
    std::string script = ss.str();
    luaL_dostring(L, script.c_str());
  }

  void onUpdate(lua_State* L) {
    std::stringstream ss;
    ss << "updateEntity('" << EntityId << "')";
    std::string script = ss.str();
//    luaL_dostring(L, script.c_str());
    
    lua_getglobal(L, "updateEntity");
    lua_pushstring(L, EntityId.c_str());

    constexpr int argumentCount = 1;
    constexpr int resultCount = 0;
    constexpr int messageHandlerIndex = 0;
    if (lua_pcall(L, argumentCount, resultCount, messageHandlerIndex) != LUA_OK) {
        luaL_error(L, "Could not update entity: %s", lua_tostring(L, -1));
    }
  }
};

struct ScriptClass {
  std::string Type;

  ScriptInstance instantiate(lua_State* L, const std::string& entityId) {
    std::stringstream ss;
    ss << "createEntity(" << Type << ":new { entityId = '" << entityId << "' })";
    std::string script = ss.str();
    luaL_dostring(L, script.c_str());
    ScriptInstance instance{};
    instance.EntityId = entityId;
    return instance;
  }
};

int main() {
	printf("Hello World\n");

	lua_State* L = luaL_newstate();
	printf("Created Lua state\n");

  luaL_openlibs(L);
	printf("Opened Lua standard libraries\n");

  lua_register(L, "printNative", printNative);
  printf("Registered native functions in Lua\n");

  luaL_dofile(L, "scripts");
  printf("Loaded Lua scripts\n");

  luaL_dostring(L, "log('Hello World from Lua')");
  
  ScriptComponent scriptComponent{};
  scriptComponent.Type = "Player";

  Entity entity{};
  entity.Id = "foobar123";
  entity.ScriptComponent = scriptComponent;

  ScriptClass scriptClass{};
  scriptClass.Type = scriptComponent.Type;

  std::vector<ScriptInstance> instances;
  instances.push_back(scriptClass.instantiate(L, entity.Id));
  instances.push_back(scriptClass.instantiate(L, "foo"));
  instances.push_back(scriptClass.instantiate(L, "bar"));
//  luaL_dostring(L, "printEntities()");
  
  lua_getglobal(L, "printEntities");
  if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
    luaL_error(L, "Could not update entity: %s", lua_tostring(L, -1));
  }
  

  for (auto& instance : instances) {
    instance.onCreate(L);
  }
  for (auto& instance : instances) {
    instance.onUpdate(L);
  }


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
