cmake_minimum_required(VERSION 3.10)

project(webcrown)

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
