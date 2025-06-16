cmake_minimum_required(VERSION 3.10)
project(CppServer)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system thread)

add_executable(CppServer main.cpp)

target_include_directories(CppServer PRIVATE ${Boost_INCLUDE_DIRS} include)
target_link_libraries(CppServer PRIVATE ${Boost_LIBRARIES})
