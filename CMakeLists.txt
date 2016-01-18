cmake_minimum_required(VERSION 3.0)
project(CG1_Framework)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(extern/glfw)

option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(extern/assimp)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /EHsc")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-function -Wno-unused-parameter -Wextra -Wpedantic -std=c++14")
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()
endif()

get_filename_component(CG1_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR} REALPATH)

set(CG1_WINDOW_WIDTH "1280" CACHE STRING "Application main window width.")
set(CG1_WINDOW_HEIGHT "720" CACHE STRING "Application main window height.")
set(CG1_SHADER_BASE_PATH "\"${CG1_ROOT_DIR}/cg1/shaders/\"")
set(CG1_RESOURCE_BASE_PATH "\"${CG1_ROOT_DIR}/cg1/resources/\"")

configure_file("cg1.h.in"
               "cg1/cg1.h")

include_directories(${PROJECT_BINARY_DIR}/cg1/
                    cg1/
                    extern/assimp/include/
                    extern/glad/include/
                    extern/glfw/include/
                    extern/glm/
                    extern/stb/
                    extern/imgui/)

file(GLOB EXTERN_SOURCES extern/glad/src/glad.c extern/imgui/imgui.cpp extern/imgui/imgui_draw.cpp)
file(GLOB PROJECT_HEADERS ${PROJECT_BINARY_DIR}/cg1/*.h
                          cg1/*.h
                          cg1/core/*.h
                          cg1/gfx/*.h
                          cg1/scenes/*.h)
file(GLOB PROJECT_SOURCES cg1/*.cpp
                          cg1/core/*.cpp
                          cg1/gfx/*.cpp
                          cg1/scenes/*.cpp)
file(GLOB PROJECT_SHADERS cg1/shaders/*.comp
                          cg1/shaders/*.frag
                          cg1/shaders/*.geom
                          cg1/shaders/*.vert)
file(GLOB PROJECT_CONFIGS CMakeLists.txt
                          Readme.md
                         .gitattributes
                         .gitignore
                         .gitmodules)

source_group("Headers" FILES ${PROJECT_HEADERS})
source_group("Shaders" FILES ${PROJECT_SHADERS})
source_group("Sources" FILES ${PROJECT_SOURCES})
source_group("Extern" FILES ${EXTERN_SOURCES})

add_definitions(-DGLFW_INCLUDE_NONE
                -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS}
                               ${PROJECT_SHADERS} ${PROJECT_CONFIGS}
                               ${EXTERN_SOURCES})
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC assimp glfw
                      ${GLFW_LIBRARIES} ${GLAD_LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
