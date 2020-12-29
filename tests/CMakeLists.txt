cmake_minimum_required(VERSION 3.10)

project(webcrown_tests)

file(GLOB_RECURSE
  SOURCE_FILES
  ${CMAKE_PROJECT_DIR}*.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/submodules/googletest/googletest/include)
include_directories(${CMAKE_SOURCE_DIR}/submodules/googletest/googlemock/include)
include_directories(${CMAKE_SOURCE_DIR}/src/include)

add_executable(webcrown_tests ${SOURCE_FILES})

target_link_libraries(webcrown_tests
  PUBLIC
    gtest
    gmock
)