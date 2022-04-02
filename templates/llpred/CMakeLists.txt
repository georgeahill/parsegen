cmake_minimum_required (VERSION 3.10)
project (Parser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Release)

add_executable(parse template.cpp)
