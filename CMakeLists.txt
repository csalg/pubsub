cmake_minimum_required(VERSION 3.14)
project(pubsub)

set(CMAKE_CXX_STANDARD 14)

add_executable(pubsub main.cpp common/generator.h common/generator.cpp common/data_structure.h common/chrono_time.h common/printer.h common/util.h common/util.cpp common/printer.cpp algorithms/rein.h algorithms/rein.cpp algorithms/siena.h algorithms/siena.cpp algorithms/htree.h algorithms/htree.cpp algorithms/opIndex.h algorithms/opIndex.cpp algorithms/tama.h algorithms/tama.cpp)