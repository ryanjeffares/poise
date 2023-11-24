cmake_minimum_required(VERSION 3.24)
project(poise)

include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY  https://github.com/fmtlib/fmt.git
    GIT_TAG         master
    GIT_SHALLOW     ON
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY  https://github.com/catchorg/Catch2.git
    GIT_TAG         v3.4.0 # or a later release
    GIT_SHALLOW     ON
)
FetchContent_MakeAvailable(Catch2)

set(POISE_COMPILE_DEFINITIONS BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED=1)

if(MSVC)
    list(APPEND POISE_COMPILE_DEFINITIONS POISE_MSVC)
    set(POISE_COMPILE_OPTIONS /W4 /WX /external:templates- /external:W0 /std:c++latest)
else()
    list(APPEND POISE_COMPILE_DEFINITIONS POISE_UNIX)
    set(POISE_COMPILE_OPTIONS -Wall -Werror -Wpedantic -Wextra -Wconversion -std=gnu++2b)
endif()
    
if (CMAKE_BUILD_TYPE MATCHES "Debug")
    list(APPEND POISE_COMPILE_DEFINITIONS POISE_DEBUG)
endif()

if (POISE_BOOST_PATH)
    set(POISE_INCLUDE_DIRECTORIES ${POISE_BOOST_PATH})
endif()

add_subdirectory(src poise)
add_subdirectory(tests poise_tests)
