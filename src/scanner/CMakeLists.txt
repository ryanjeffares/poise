add_library(poise_scanner Scanner.cpp Token.cpp)
target_link_libraries(poise_scanner PRIVATE fmt::fmt)