find_package(Python REQUIRED COMPONENTS Interpreter)

install(TARGETS pppp_c LIBRARY DESTINATION pppp RUNTIME DESTINATION pppp)

if(TARGET fosu_engine_avx2)
  install(TARGETS fosu_engine_avx2 LIBRARY DESTINATION pppp RUNTIME DESTINATION pppp)
elseif(TARGET fosu_engine_neon)
  install(TARGETS fosu_engine_neon LIBRARY DESTINATION pppp RUNTIME DESTINATION pppp)
endif()
