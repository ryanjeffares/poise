add_library(poise_runtime Op.cpp Value.cpp Vm.cpp Vm_RegisterNatives.cpp NativeFunction.cpp)

target_compile_definitions(poise_runtime PRIVATE ${POISE_COMPILE_DEFINITIONS})
target_compile_options(poise_runtime PRIVATE ${POISE_COMPILE_OPTIONS})
target_include_directories(poise_runtime PRIVATE ${POISE_INCLUDE_DIRECTORIES})
target_link_libraries(poise_runtime PRIVATE fmt::fmt)