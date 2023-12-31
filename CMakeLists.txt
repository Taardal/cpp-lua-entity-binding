cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 20)

project(cpp-lua-entity-binding)

add_executable(
        ${PROJECT_NAME} 
        src/main.cpp 
        src/printlua.cpp 
        src/luafunction.cpp
)

set(INSTALL_DIR ${PROJECT_SOURCE_DIR})
set(BIN_DIR "${PROJECT_SOURCE_DIR}/bin")

set(RUNTIME_NAME "app")
set(RUNTIME_NAME_DEBUG "${RUNTIME_NAME}_debug")
set(RUNTIME_NAME_RELEASE "${RUNTIME_NAME}_release")

set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BIN_DIR}/debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BIN_DIR}/release"
        RUNTIME_OUTPUT_NAME ${RUNTIME_NAME}
        RUNTIME_OUTPUT_NAME_DEBUG ${RUNTIME_NAME_DEBUG}
        RUNTIME_OUTPUT_NAME_RELEASE ${RUNTIME_NAME_RELEASE}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_DIR})
install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug DESTINATION ${INSTALL_DIR})
install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Release DESTINATION ${INSTALL_DIR})

find_package(Lua REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${LUA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${LUA_LIBRARY})

