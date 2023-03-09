cmake_minimum_required(VERSION 3.12)

project(webcrown LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

option(ENABLE_ORM "" ON)
option(ENABLE_TESTS "" ON)
option(ENABLE_EXAMPLES "" ON)

set (CMAKE_CXX_FLAGS "-Werror=return-type")

include_directories(${PROJECT_SOURCE_DIR}/include)

include_directories(${PROJECT_SOURCE_DIR}/.deps/asio/asio/include)
include_directories(${PROJECT_SOURCE_DIR}/.deps/date/include)
include_directories(${PROJECT_SOURCE_DIR}/.deps/json/include)
include_directories(${PROJECT_SOURCE_DIR}/.deps/spdlog/include)
include_directories(${PROJECT_SOURCE_DIR}/.deps/reflcpp/include)

file(GLOB_RECURSE
  SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/include/*.hpp
    ${PROJECT_SOURCE_DIR}/include/*.h
    ${PROJECT_SOURCE_DIR}/include/*.cpp
)

# submodules
#add_subdirectory(submodules/googletest)

if (ENABLE_TESTS)
#add_subdirectory(tests)
endif()

#add_subdirectory(examples/authorization)
#add_subdirectory(examples/simple_route)

if (ENABLE_EXAMPLES)
  #add_subdirectory(examples/orm)
endif()

add_library(webcrown STATIC ${SOURCE_FILES})
