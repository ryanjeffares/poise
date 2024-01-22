add_library(
    poise-runtime

    memory/StringInterner.cpp
    NamespaceManager.cpp
    NativeFunction.cpp
    Op.cpp
    Types.cpp
    Value.cpp
    Vm.cpp
    Vm_RegisterNatives.cpp
)

target_compile_definitions(poise-runtime PRIVATE ${POISE_COMPILE_DEFINITIONS})
target_compile_options(poise-runtime PRIVATE ${POISE_COMPILE_OPTIONS})
target_include_directories(poise-runtime PRIVATE ${POISE_INCLUDE_DIRECTORIES})
target_link_libraries(poise-runtime PRIVATE fmt::fmt poise-objects)
