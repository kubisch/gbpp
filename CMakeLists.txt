cmake_minimum_required(VERSION 3.28.0)
project(gbpp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

include_directories(
  src
)

set(SRC_FILES
  src/main.cpp
)

set(LIB_FILES
  src/dmg.h
  src/dmg.cpp
)

# set(HEADER_FILES
#   src/dmg.h
# )

add_library(gbpp_internal STATIC
  ${LIB_FILES}
)

add_executable(gbpp
  ${SRC_FILES}
)
target_link_libraries(gbpp PRIVATE gbpp_internal)

file(GLOB TEST_SOURCES tests/*.cpp)

foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)

    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} PRIVATE gbpp_internal)

    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()