project(api_admin LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE
    SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
        ${CMAKE_BINARY_DIR}/bin
        )

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
        ${CMAKE_BINARY_DIR}/lib
        )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_BINARY_DIR}/bin
        )


include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(refl-cpp REQUIRED)
find_package(asio REQUIRED)
find_package(date REQUIRED)
find_package(libpqxx REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)

find_package(argon2 REQUIRED)
find_package(inja REQUIRED)
find_package(Boost REQUIRED)

include_directories(${refl-cpp_INCLUDE_DIRS})
include_directories(${asio_INCLUDE_DIRS})
include_directories(${date_INCLUDE_DIRS})
include_directories(${libpqxx_INCLUDE_DIRS})
include_directories(${nlohmann_json_INCLUDE_DIRS})
include_directories(${argon2_INCLUDE_DIRS})
include_directories(${inja_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS} 1.81 REQUIRED COMPONENTS url)

include_directories(${fmt_INCLUDE_DIRS})
include_directories(${spdlog_INCLUDE_DIRS})

add_executable(api_admin ${SOURCE_FILES})

target_link_libraries(api_admin
    webcrown
    ${libpqxx_LIBRARIES}
    ${cpp-jwt_LIBRARIES}
    ${argon2_LIBRARIES}
    ${fmt_LIBRARIES}
    ${spdlog_LIBRARIES}
    Boost::url
)