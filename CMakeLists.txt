cmake_minimum_required(VERSION 3.4)
project(tuft)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(NLOHMANN_JSON_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../json/src/" CACHE PATH "Path to json.hpp from nlohmann's json library")

include_directories("%{CMAKE_CURRENT_SOURCE_DIR}/")

find_package(Boost 1.60.0 REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})

set(SOURCE_FILES main.cpp tuft.hpp)
add_executable(tuft ${SOURCE_FILES})
