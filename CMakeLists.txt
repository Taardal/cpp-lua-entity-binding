cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 20)

project(luapoc)

add_executable(${PROJECT_NAME} main.cpp)

find_package(Lua REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${LUA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${LUA_LIBRARY})
