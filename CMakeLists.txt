cmake_minimum_required(VERSION 3.12)

project(webcrown LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

set (CMAKE_CXX_FLAGS "-Werror=return-type")

include_directories(${PROJECT_SOURCE_DIR}/third_party/asio/include)
include_directories(${PROJECT_SOURCE_DIR}/third_party/nlohmann)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/submodules/spdlog/include)

file(GLOB_RECURSE
  SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/include/*.hpp
    ${PROJECT_SOURCE_DIR}/include/*.h
    #${PROJECT_SOURCE_DIR}/include/*.inl
    ${PROJECT_SOURCE_DIR}/include/*.cpp
)

#set_source_files_properties(${SOURCE_FILES} PROPERTIES LANGUAGE CXX)

# submodules
add_subdirectory(submodules/spdlog)
add_subdirectory(submodules/googletest)
add_subdirectory(tests)
add_subdirectory(examples/simple_route)
add_subdirectory(examples/multiple_buffers_receive)

# External Libs
## OpenSSL
if (APPLE)
    # This is a bug in CMake that causes it to prefer the system version over
    # the one in the specified ROOT folder.
    set(OPENSSL_ROOT_DIR ${OPENSSL_ROOT_DIR} /usr/local/opt/openssl@1.1/)
    set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib CACHE FILEPATH "" FORCE)
    set(OPENSSL_SSL_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libssl.dylib CACHE FILEPATH "" FORCE)
    include_directories(${OPENSSL_ROOT_DIR}/include)
endif()
find_package(OpenSSL REQUIRED)

## PThread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(webcrown STATIC ${SOURCE_FILES})
