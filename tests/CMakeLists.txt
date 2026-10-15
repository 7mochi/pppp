function(pppp_add_test name label)
  add_executable(test_${name} test_${name}.cpp)
  target_compile_features(test_${name} PRIVATE cxx_std_98)
  target_compile_definitions(test_${name} PRIVATE PPPP_TEST_RESOURCES="${CMAKE_CURRENT_SOURCE_DIR}/resources"
    PPPP_TEST_PARITY="${CMAKE_CURRENT_SOURCE_DIR}/parity/parity.json")
  target_link_libraries(test_${name} PRIVATE pppp_test_main pppp_internal pppp_warnings pppp_numerics ${ARGN})
  if(WIN32)
    add_custom_command(TARGET test_${name} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:test_${name}> $<TARGET_RUNTIME_DLLS:test_${name}>
      COMMAND_EXPAND_LISTS)
  endif()
  add_test(NAME ${name} COMMAND test_${name} ${PPPP_TEST_ARGS})
  set_tests_properties(${name} PROPERTIES LABELS ${label} TIMEOUT 300)
endfunction()

function(pppp_place_engine test_name)
  if(TARGET fosu_engine_avx2)
    set(engine $<TARGET_FILE:fosu_engine_avx2>)
  elseif(TARGET fosu_engine_neon)
    set(engine $<TARGET_FILE:fosu_engine_neon>)
  else()
    return()
  endif()
  add_custom_command(TARGET test_${test_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${engine} $<TARGET_FILE_DIR:test_${test_name}>)
  add_test(NAME ${test_name}_scalar COMMAND test_${test_name})
  set_tests_properties(${test_name}_scalar PROPERTIES LABELS library TIMEOUT 300
    ENVIRONMENT "FOSU_BACKEND=scalar")
endfunction()

if(PPPP_WITH_FOSU)
  pppp_add_test(beatmap library pppp_fosu fosu)
  pppp_add_test(timed_and_strains library pppp_fosu)
  pppp_add_test(mod_behaviour library pppp_fosu)
  pppp_add_test(backend library pppp_fosu)
  pppp_place_engine(backend)
  pppp_add_test(linq library)

  pppp_add_test(parity_decode library pppp_fosu pppp_cjson)
  pppp_place_engine(parity_decode)
  pppp_add_test(parity_difficulty library pppp_fosu pppp_cjson)
  pppp_place_engine(parity_difficulty)
  pppp_add_test(parity_performance library pppp_fosu pppp_cjson)
  pppp_place_engine(parity_performance)
endif()

if(PPPP_WITH_FOSU AND PPPP_BUILD_CAPI)
  # --success names every test case as it runs, so a case that hangs and is killed by the timeout
  # above can be told apart from the ones before it in the CI log.
  set(PPPP_TEST_ARGS --success)
  pppp_add_test(capi library pppp_fosu pppp_c pppp_cjson)
  unset(PPPP_TEST_ARGS)
endif()

if(PPPP_WITH_FOSU AND PPPP_INSTALL)
  add_test(NAME install_consumer COMMAND ${CMAKE_COMMAND}
    -DPPPP_BINARY=${CMAKE_BINARY_DIR}
    -DPPPP_CONSUMER=${CMAKE_CURRENT_SOURCE_DIR}/install/consumer
    -DPPPP_WORK=${CMAKE_BINARY_DIR}/install-consumer-test
    -DPPPP_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DPPPP_C_COMPILER=${CMAKE_C_COMPILER}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/install/run.cmake)
  set_tests_properties(install_consumer PROPERTIES LABELS library TIMEOUT 300)
endif()
