add_executable(poise main.cpp)

add_subdirectory(compiler)
add_subdirectory(objects)
add_subdirectory(runtime)
add_subdirectory(scanner)

target_link_libraries(poise PUBLIC fmt::fmt poise_compiler poise_objects poise_runtime poise_scanner)
