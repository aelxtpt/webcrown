cmake_minimum_required(VERSION 3.10)

project(WebCrownAdmin)

set(EMSCRIPTEN_ROOT_PATH "/home/aex/code/emsdk")

list(APPEND CMAKE_MODULE_PATH "${EMSCRIPTEN_ROOT_PATH}/upstream/emscripten/cmake")


# Set on cmake args

# To WORK, need pass
# -GNinja
# -DCMAKE_BUILD_TYPE:STRING=Debug
# -DCMAKE_TOOLCHAIN_FILE=/home/alexlima/code/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
# -DCMAKE_SYSTEM_PROCESSOR=x86_64
# -DEMSCRIPTEN_SYSTEM_PROCESSOR=x86_64
# -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
# -DCMAKE_EXE_LINKER_FLAGS=-s USE_GLFW=3

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type -Wno-implicit-const-int-float-conversion")


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
        ${CMAKE_BINARY_DIR}/bin
        )

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
        ${CMAKE_BINARY_DIR}/lib
        )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_BINARY_DIR}/bin
        )

file(GLOB_RECURSE
    SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/include/*.hpp
    ${PROJECT_SOURCE_DIR}/source/*.cpp)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/imgui)

# webcrown deps
include_directories(${PROJECT_SOURCE_DIR}/../../include)
include_directories(${PROJECT_SOURCE_DIR}/../../.deps/date/include)
include_directories(${PROJECT_SOURCE_DIR}/../../.deps/reflcpp/include)
include_directories(${PROJECT_SOURCE_DIR}/../../.deps/libpqxx/include)

include_directories(${EMSCRIPTEN_ROOT_PATH}/upstream/emscripten/cache/sysroot/include)
include_directories(${EMSCRIPTEN_ROOT_PATH}/upstream/emscripten/cache/sysroot/include/emscripten)

configure_file(${PROJECT_SOURCE_DIR}/front_static/index.html
    ${CMAKE_CURRENT_BINARY_DIR}/bin/index.html COPYONLY)

# Deps
include_directories(${PROJECT_SOURCE_DIR}/include/imgui/implot)
add_subdirectory(.deps/libpqxx)

#find_package(OpenGL REQUIRED)

#find_package(glfw3 REQUIRED)

set(CMAKE_EXECUTABLE_SUFFIX ".html")

add_executable(WebCrownAdmin source/webcrown_admin.cpp ${SOURCE_FILES})

set_target_properties(WebCrownAdmin PROPERTIES LINK_FLAGS "--preload-file ${PROJECT_SOURCE_DIR}/resources")

target_link_libraries(WebCrownAdmin
    glfw
    dl
    GL
    pthread
    pqxx
)
