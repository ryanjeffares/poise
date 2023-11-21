add_library(poise_runtime Op.cpp Value.cpp Vm.cpp)
target_link_libraries(poise_runtime PRIVATE fmt::fmt)