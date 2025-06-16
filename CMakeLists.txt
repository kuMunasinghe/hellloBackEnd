cmake_minimum_required(VERSION 3.10)
project(CppServer)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

add_executable(CppServer main.cpp)

target_include_directories(CppServer PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(CppServer PRIVATE ${Boost_LIBRARIES})

# For JSON library
include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp)
FetchContent_MakeAvailable(json)
target_include_directories(CppServer PRIVATE ${json_SOURCE_DIR})
