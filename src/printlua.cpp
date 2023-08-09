#include <lua.hpp>
#include <iostream>

void printLua(lua_State* L, const std::string& tag)
{
    if (!tag.empty())
    {
        printf("%s\n", tag.c_str());
    }
    printf("-- lua stack\n");
    int stackSize = lua_gettop(L);
    if (stackSize == 0)
    {
        printf("--   empty\n");
    }
    else
    {
        for (int i = 1; i <= stackSize; i++)
        {
            int stackIndex = i * -1;
            printf("--   [%d] ", stackIndex);
            if (lua_isuserdata(L, stackIndex))
            {
                printf("userdata\n");
            }
            else if (lua_islightuserdata(L, stackIndex))
            {
                printf("light userdata\n");
            }
            else if (lua_istable(L, stackIndex))
            {
                printf("table\n");
            }
            else if (lua_isfunction(L, stackIndex))
            {
                printf("function\n");
            }
            else if (lua_iscfunction(L, stackIndex))
            {
                printf("cfunction\n");
            }
            else if (lua_isthread(L, stackIndex))
            {
                printf("thread\n");
            }
            else if (lua_isnumber(L, stackIndex))
            {
                printf("number\n");
            }
            else if (lua_isstring(L, stackIndex))
            {
                printf("string\n");
            }
            else if (lua_isnone(L, stackIndex))
            {
                printf("none\n");
            }
            else if (lua_isnil(L, stackIndex))
            {
                printf("nil\n");
            }
            else
            {
                printf("unknown type\n");
            }
        }
    }
    printf("-- end\n");
}
