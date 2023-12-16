add_executable(poise_tests Test_Objects.cpp Test_RunFiles.cpp Test_Values.cpp)

target_compile_definitions(poise_tests PRIVATE ${POISE_COMPILE_DEFINITIONS})
target_compile_options(poise_tests PRIVATE ${POISE_COMPILE_OPTIONS})
target_include_directories(poise_tests PRIVATE ${POISE_INCLUDE_DIRECTORIES})

target_link_libraries(poise_tests PRIVATE Catch2::Catch2WithMain fmt::fmt poise_compiler poise_objects poise_runtime poise_scanner)