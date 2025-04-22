cmake_minimum_required(VERSION 3.28.0)
project(gbpp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
  src
)

set(SRC_FILES
  src/main.cpp
  src/dmg.cpp
)

# set(HEADER_FILES
#   src/dmg.h
# )

add_executable(gbpp
  ${SRC_FILES}
)