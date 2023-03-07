cmake_minimum_required(VERSION 3.12)

project(orm_example LANGUAGES C CXX ASM)

include_directories(../../include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

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

add_executable(orm_example entrypoint.cpp)

target_link_libraries(orm_example
    PUBLIC
    Threads::Threads
    OpenSSL::SSL
    webcrown)