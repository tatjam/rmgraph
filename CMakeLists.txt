cmake_minimum_required (VERSION 3.2.3)

# Define basic compiler and CMake configuration

project(rmdev)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)


find_package(Threads REQUIRED)

add_executable(rmdev src/Main.cpp)
include_directories(rmdev PUBLIC src src/clatexmath)
target_link_libraries(rmdev Threads::Threads)
target_compile_definitions(rmdev PUBLIC DEV=true)
target_compile_options(rmdev PUBLIC -fno-elide-constructors)


project(rmdev-native)

set (CMAKE_CXX_STANDARD 17)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)

add_executable(rmdev-native src/Main.cpp)
include_directories(rmdev-native PUBLIC src)
target_link_libraries(rmdev-native Threads::Threads)
target_compile_definitions(rmdev-native PUBLIC REMARKABLE=1)
target_compile_options(rmdev-native PUBLIC -Ofast)

