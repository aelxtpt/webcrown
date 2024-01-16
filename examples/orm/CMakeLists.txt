
project(orm_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")

find_package(libpqxx REQUIRED)

link_directories(/home/aex/code/libpqxx/src)


add_executable(orm_example entrypoint.cpp)



target_link_libraries(orm_example
    webcrown
    ${libpqxx_LIBRARIES}

)