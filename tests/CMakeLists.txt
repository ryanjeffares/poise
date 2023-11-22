add_executable(poise_tests PoiseTests.cpp)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(poise_tests PRIVATE POISE_DEBUG)
endif()

target_include_directories(poise_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../third_party)

if (MSVC)
    target_compile_definitions(poise_tests PRIVATE POISE_MSVC)
    target_compile_options(poise_tests PRIVATE /W4 /WX /external:templates- /external:W0 /std:c++latest)
else()
    target_compile_definitions(poise_tests PRIVATE POISE_UNIX)
    target_compile_options(poise_tests PRIVATE -Wall -Werror -Wpedantic -Wextra -Wconversion -std=gnu++2b)
endif()

target_link_libraries(poise_tests PRIVATE Catch2::Catch2WithMain fmt::fmt poise_compiler poise_objects poise_runtime poise_scanner)