cmake_minimum_required(VERSION 3.26)
project(pppp_install_consumer CXX)

find_package(pppp CONFIG REQUIRED)

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE pppp::fosu)
pppp_place_engine(consumer)
