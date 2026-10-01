option(PPPP_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

add_library(pppp_warnings INTERFACE)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(pppp_warnings INTERFACE
    -Wall -Wextra -Wpedantic -Wshadow -Wundef -Wcast-align -Wno-long-long)
  if(PPPP_WARNINGS_AS_ERRORS)
    target_compile_options(pppp_warnings INTERFACE -Werror)
  endif()
elseif(MSVC)
  target_compile_options(pppp_warnings INTERFACE /W4)
  if(PPPP_WARNINGS_AS_ERRORS)
    target_compile_options(pppp_warnings INTERFACE /WX)
  endif()
endif()
