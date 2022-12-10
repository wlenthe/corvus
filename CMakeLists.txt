cmake_minimum_required(VERSION 3.1)

project(corvus)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(wif source/wif.cpp)
add_library(draft source/cell.cpp)

add_executable(read_wif test/read_wif.cpp)
target_link_libraries(read_wif wif)

add_executable(deduce_draft test/deduce_draft.cpp)
target_link_libraries(deduce_draft draft)
