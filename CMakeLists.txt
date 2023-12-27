project(reversi)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE reversi_srcs "reversi/src/*.cc")
add_executable(reversi "${reversi_srcs}")
target_include_directories(reversi PUBLIC "reversi/include")