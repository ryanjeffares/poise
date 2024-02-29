add_library(
    poise-objects

    Exception.cpp
    Function.cpp
    iterables/hashables/Dict.cpp
    iterables/hashables/Hashable.cpp
    iterables/hashables/Set.cpp
    iterables/Iterable.cpp
    iterables/Iterator.cpp
    iterables/List.cpp
    iterables/Range.cpp
    iterables/Tuple.cpp
    Object.cpp
    Struct.cpp
    Type.cpp
)

target_compile_definitions(poise-objects PRIVATE ${POISE_COMPILE_DEFINITIONS})
target_compile_options(poise-objects PRIVATE ${POISE_COMPILE_OPTIONS})
target_include_directories(poise-objects PRIVATE ${POISE_INCLUDE_DIRECTORIES})
target_link_libraries(poise-objects PRIVATE fmt::fmt)
