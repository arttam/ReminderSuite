cmake_minimum_required(VERSION 3.6)
project(Server)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost 1.62.0 REQUIRED system)
if (NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()

set(SOURCE_FILES
        src/main.cpp
        src/server.h src/server.cpp
        src/session.h src/session.cpp)

add_executable(Server ${SOURCE_FILES})
target_link_libraries(Server ${Boost_LIBRARIES})
target_link_libraries(Server pthread)
target_link_libraries(Server dl)