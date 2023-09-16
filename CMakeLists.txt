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

add_subdirectory(src poise)
