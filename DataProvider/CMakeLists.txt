cmake_minimum_required(VERSION 3.6)
project(DataProvider)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        src/DBLibrary.h src/DBLibrary.cpp
        src/DBTextFile/DBTextFile.h src/DBTextFile/DBTextFile.cpp
        src/DBTextFile/entry.h src/DBTextFile/entry.cpp
        src/DBTextFile/filehandler.h src/DBTextFile/filehandler.cpp)

add_library(DataProvider SHARED ${SOURCE_FILES})