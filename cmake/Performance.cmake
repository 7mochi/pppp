add_library(pppp_performance INTERFACE)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(pppp_performance INTERFACE
    -fno-exceptions -fno-rtti
    -ffunction-sections -fdata-sections
    -pipe)
  target_link_options(pppp_performance INTERFACE -Wl,--gc-sections)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
  if(ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
elseif(MSVC)
  target_compile_options(pppp_performance INTERFACE /GR- /EHs-c-)
endif()
