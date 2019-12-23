cmake_minimum_required(VERSION 3.14)
project(pubsub)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Wall -O3")

add_executable(
        pubsub
        main.cpp
        common/generator.h
        common/generator.cpp
        common/data_structure.h
        common/chrono_time.h
        common/csv_parser.hpp
        common/printer.h
        common/util.h
        common/util.cpp
        common/printer.cpp
        algorithms/rein.h
        algorithms/rein.cpp
        algorithms/siena.h
        algorithms/siena.cpp
        algorithms/htree.h
        algorithms/htree.cpp
        algorithms/opIndex.h
        algorithms/opIndex.cpp
        algorithms/tama.h
        algorithms/tama.cpp
        algorithms/subscriptionForest/AVLTree.h
        algorithms/subscriptionForest/IntervalNode.h
        algorithms/subscriptionForest/CenteredIntervalTree.h
        algorithms/subscriptionForest/IntervalTree.h
        algorithms/subscriptionForest.h
        algorithms/ACTreeLogVolume.h
        params.h
        algorithms/SCTreeParallel.h algorithms/ACTreeNearestCenter.h algorithms/vari-rein.cpp algorithms/vari-siena.h algorithms/vari-siena.cpp)