cmake_minimum_required(VERSION 3.16)

project(webcrown)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR}/third_party/asio/include)
include_directories(${CMAKE_SOURCE_DIR}/src/include)


file(GLOB_RECURSE
  SOURCE_FILES
  ${CMAKE_SOURCE_DIR}/src/*.h
  ${CMAKE_SOURCE_DIR}/src/*.inl
  ${CMAKE_SOURCE_DIR}/src/*.cpp
)

find_package(OpenSSL REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(webcrown ${SOURCE_FILES})

target_link_libraries(webcrown
  Threads::Threads
  OpenSSL::SSL 
)
