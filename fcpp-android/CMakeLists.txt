# minimum required version of CMake
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
# avoid building internal tests of the FCPP library
option(FCPP_INTERNAL_TESTS "Build internal tests for FCPP." OFF)
# load the FCPP sub-module
add_subdirectory(./fcpp/src)
# CMake macro setting up the FCPP library
fcpp_setup()

# project description
project(
    android
    VERSION 0.1.0
    DESCRIPTION "Android Case Study."
)

# headers declaration
set(
    SOURCE_LIST
    ./lib/common/template_remover.cpp
    ./lib/coordination/experiment_helper.cpp
    ./lib/coordination/main.cpp
    ./lib/coordination/movement.cpp
    ./lib/coordination/past_ctl.cpp
    ./lib/coordination/slcs.cpp
    ./lib/coordination/tracker.cpp
    ./lib/coordination/traitor_detection.cpp
    ./lib/coordination/vulnerability_detection.cpp
    ./lib/configuration.cpp
    ./run/main.cpp
)
fcpp_headers(main OFF "${SOURCE_LIST}")

# executable declaration
fcpp_target(./run/simulation.cpp ON)

# test declaration
fcpp_test(./test/common/template_remover.cpp)
