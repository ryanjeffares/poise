cmake_minimum_required(VERSION 3.24)
project(poise)

include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG master
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.4.0 # or a later release
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(Catch2)

add_subdirectory(src poise)
add_subdirectory(tests poise_tests)
