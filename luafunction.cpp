#include <lua.hpp>
#include <iostream>

/**
 * Lua: A Generic Call Function
 *
 * Source:
 *   - https://www.lua.org/pil/25.3.html
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
 */
void invokeLuaFunction(lua_State* L, const char* functionName, const char* signature, ...) {
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