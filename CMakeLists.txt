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

find_package(refl-cpp REQUIRED)
find_package(asio REQUIRED)
find_package(date REQUIRED)
find_package(libpqxx REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(argon2 REQUIRED)


include_directories(${refl-cpp_INCLUDE_DIRS})
include_directories(${asio_INCLUDE_DIRS})
include_directories(${date_INCLUDE_DIRS})
include_directories(${libpqxx_INCLUDE_DIRS})
include_directories(${nlohmann_json_INCLUDE_DIRS})
include_directories(${argon2_INCLUDE_DIRS})



file(GLOB_RECURSE
  SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/include/*.hpp
    ${PROJECT_SOURCE_DIR}/include/*.h
    ${PROJECT_SOURCE_DIR}/include/*.cpp
)


file(GLOB_RECURSE 
        STATIC_FILES
        ${PROJECT_SOURCE_DIR}/include/webcrown/admin/static/*
)

foreach(file ${STATIC_FILES}) 
    get_filename_component(filename ${file} NAME)
    get_filename_component(dir ${file} DIRECTORY)
    string(REPLACE "${PROJECT_SOURCE_DIR}/include/webcrown/admin/static/" "" dir ${dir})
    configure_file(${file} ${CMAKE_BINARY_DIR}/bin/static/${dir}/${filename} COPYONLY)
endforeach()


# submodules
#add_subdirectory(submodules/googletest)

if (ENABLE_TESTS)
#add_subdirectory(tests)
endif()

#add_subdirectory(examples/authorization)
#add_subdirectory(examples/simple_route)

if (ENABLE_EXAMPLES)
  #add_subdirectory(examples/orm)
  add_subdirectory(examples/simple_api)
endif()

add_library(webcrown STATIC ${SOURCE_FILES})
target_link_libraries(webcrown
  ${fmt_LIBRARIES}
)
