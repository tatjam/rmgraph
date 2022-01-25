cmake_minimum_required (VERSION 3.2.3)

# Define basic compiler and CMake configuration

project(rmdev)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)


find_package(Threads REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(rmdev ${SOURCES})
include_directories(rmdev PUBLIC src)
target_link_libraries(rmdev Threads::Threads)

project(rmdev-native)

set (CMAKE_CXX_STANDARD 17)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)


find_package(Threads REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(rmdev-native ${SOURCES})
include_directories(rmdev-native PUBLIC src)
target_link_libraries(rmdev-native Threads::Threads)

