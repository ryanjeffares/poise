add_library(poise_objects
        iterables/hashables/Dict.cpp
        iterables/hashables/Hashable.cpp
        iterables/Iterable.cpp
        iterables/Iterator.cpp
        iterables/List.cpp
        iterables/Range.cpp
        iterables/Tuple.cpp
        Exception.cpp
        Function.cpp
        Object.cpp
        Type.cpp
)

target_compile_definitions(poise_objects PRIVATE ${POISE_COMPILE_DEFINITIONS})
target_compile_options(poise_objects PRIVATE ${POISE_COMPILE_OPTIONS})
target_include_directories(poise_objects PRIVATE ${POISE_INCLUDE_DIRECTORIES})
target_link_libraries(poise_objects PRIVATE fmt::fmt)
