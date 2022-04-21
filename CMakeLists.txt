cmake_minimum_required(VERSION 3.12)

# set the project name
project(TinyPB)

# has to be real 20 not gnu20a
set(CMAKE_CXX_STANDARD 20)

# add the executable
add_executable(TinyPB src/protobuf.cpp)
target_include_directories(TinyPB PUBLIC include)
target_link_libraries(TinyPB PUBLIC pthread gtest gtest_main)