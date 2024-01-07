add_library(poise_objects
        iterables/PoiseIterable.cpp
        iterables/PoiseIterator.cpp
        iterables/PoiseList.cpp
        iterables/PoiseRange.cpp
        iterables/PoiseTuple.cpp
        PoiseException.cpp
        PoiseFunction.cpp
        PoiseObject.cpp
        PoiseType.cpp
)

target_compile_definitions(poise_objects PRIVATE ${POISE_COMPILE_DEFINITIONS})
target_compile_options(poise_objects PRIVATE ${POISE_COMPILE_OPTIONS})
target_include_directories(poise_objects PRIVATE ${POISE_INCLUDE_DIRECTORIES})
target_link_libraries(poise_objects PRIVATE fmt::fmt)
