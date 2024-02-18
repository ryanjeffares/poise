add_executable(
    poise-tests

    Test_Memory.cpp
    Test_Objects.cpp
    Test_RunFiles.cpp
    Test_Utils.cpp
    Test_Values.cpp
)

target_compile_definitions(poise-tests PRIVATE ${POISE_COMPILE_DEFINITIONS})
target_compile_options(poise-tests PRIVATE ${POISE_COMPILE_OPTIONS})
target_include_directories(poise-tests PRIVATE ${POISE_INCLUDE_DIRECTORIES})

target_link_libraries(poise-tests PRIVATE Catch2::Catch2WithMain fmt::fmt poise-compiler poise-objects poise-runtime poise-scanner)
