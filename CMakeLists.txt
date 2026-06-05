cmake_minimum_required(VERSION 3.16)
project(parallel_regex_engine)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenMP REQUIRED)

add_executable(parallel_regex_engine main.cpp
        RegexParser.cpp
        RegexParser.h
        Automata.cpp
        Automata.h
        PaREMMatcher.cpp
        PaREMMatcher.h)

target_link_libraries(parallel_regex_engine OpenMP::OpenMP_CXX)

