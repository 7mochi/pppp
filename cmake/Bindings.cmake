add_library(pppp_bindings INTERFACE)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(pppp_bindings INTERFACE -fexceptions)
elseif(MSVC)
  target_compile_options(pppp_bindings INTERFACE /EHsc)
endif()
