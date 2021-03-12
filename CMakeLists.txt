cmake_minimum_required(VERSION 3.16)

project(webcrown)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (CMAKE_CXX_FLAGS "-Werror=return-type")

include_directories(${CMAKE_SOURCE_DIR}/third_party/asio/include)
include_directories(${CMAKE_SOURCE_DIR}/src/include)
include_directories(${CMAKE_SOURCE_DIR}/submodules/spdlog/include)

file(GLOB_RECURSE
  SOURCE_FILES
  ${CMAKE_SOURCE_DIR}/src/*.h
  ${CMAKE_SOURCE_DIR}/src/*.inl
  ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# submodules
add_subdirectory(submodules/spdlog)
add_subdirectory(submodules/googletest)
add_subdirectory(tests)

# External Libs
## OpenSSL
find_package(OpenSSL REQUIRED)

## PThread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(webcrown ${SOURCE_FILES})

target_link_libraries(webcrown
  Threads::Threads
  OpenSSL::SSL 
)
