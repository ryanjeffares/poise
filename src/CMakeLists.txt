add_executable(poise
  main.cpp
  scanner/Scanner.cpp
  scanner/Token.cpp
  compiler/Compiler.cpp
)

find_package(fmt REQUIRED)

set_target_properties(poise PROPERTIES 
  CXX_STANDARD 23
)

target_compile_options(poise PRIVATE -Wall -Werror -Wpedantic -Wextra -Wconversion)
target_link_libraries(poise PRIVATE fmt::fmt)
