cmake_minimum_required(VERSION 3.12)

project(webcrown LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

option(ENABLE_ADMIN, "" OFF)
option(ENABLE_ORM "" ON)
option(ENABLE_TESTS "" ON)

set (CMAKE_CXX_FLAGS "-Werror=return-type")

include_directories(${PROJECT_SOURCE_DIR}/third_party/asio/include)
include_directories(${PROJECT_SOURCE_DIR}/third_party/json/nlohmann)
include_directories(${PROJECT_SOURCE_DIR}/third_party/reflcpp/include)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/submodules/spdlog/include)

file(GLOB_RECURSE
  SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/include/*.hpp
    ${PROJECT_SOURCE_DIR}/include/*.h
    #${PROJECT_SOURCE_DIR}/include/*.inl
    ${PROJECT_SOURCE_DIR}/include/*.cpp
)

if (NOT ENABLE_ADMIN)
file(REMOVE_RECURSE
  SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/include/webcrown/admin/*.hpp
    ${PROJECT_SOURCE_DIR}/include/webcrown/admin/*.h
    ${PROJECT_SOURCE_DIR}/include/webcrown/admin/*.cpp
    )
endif()

# submodules
add_subdirectory(submodules/googletest)

if (ENABLE_TESTS)
#add_subdirectory(tests)
endif()

#add_subdirectory(examples/authorization)
#add_subdirectory(examples/simple_route)

add_library(webcrown STATIC ${SOURCE_FILES})
