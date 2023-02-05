cmake_minimum_required(VERSION 3.6)

project(dumb_buffer_example)

add_executable(dumb_buffer dumb_buffer.cpp)
target_link_libraries(dumb_buffer preview)
target_include_directories(dumb_buffer PRIVATE ../include)
