add_executable(poise main.cpp)

add_subdirectory(compiler)
add_subdirectory(objects)
add_subdirectory(runtime)
add_subdirectory(scanner)

target_compile_definitions(poise PRIVATE ${POISE_COMPILE_DEFINITIONS})
target_compile_options(poise PRIVATE ${POISE_COMPILE_OPTIONS})
target_include_directories(poise PRIVATE ${POISE_INCLUDE_DIRECTORIES})

target_link_libraries(poise PUBLIC fmt::fmt poise_compiler poise_objects poise_runtime poise_scanner)
