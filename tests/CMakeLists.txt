include(FetchContent)

if(APPLE)
  set(PPPP_DOCTEST_PATCH
    PATCH_COMMAND patch -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/patches/doctest-arm64.patch)
endif()

FetchContent_Declare(doctest
  URL https://github.com/doctest/doctest/archive/cd1d7474374c8cebed5c609564cdaa2f5184be25.tar.gz
  URL_HASH SHA256=cd9d23bba8247dfb47b132d7fb16414db72c451b1c51dd15d332dd064424250f
  SOURCE_SUBDIR doctest
  ${PPPP_DOCTEST_PATCH})
FetchContent_MakeAvailable(doctest)

add_library(pppp_doctest INTERFACE)
target_include_directories(pppp_doctest SYSTEM INTERFACE ${doctest_SOURCE_DIR}/doctest)
target_compile_definitions(pppp_doctest INTERFACE DOCTEST_CONFIG_NO_POSIX_SIGNALS)

add_library(pppp_test_main OBJECT main.cpp)
target_compile_features(pppp_test_main PRIVATE cxx_std_98)
target_link_libraries(pppp_test_main PUBLIC pppp_doctest)

function(pppp_add_test name label)
  add_executable(test_${name} test_${name}.cpp)
  target_compile_features(test_${name} PRIVATE cxx_std_98)
  target_compile_definitions(test_${name} PRIVATE PPPP_TEST_RESOURCES="${CMAKE_CURRENT_SOURCE_DIR}/resources")
  target_link_libraries(test_${name} PRIVATE pppp_test_main pppp_internal pppp_warnings pppp_numerics ${ARGN})
  if(WIN32)
    add_custom_command(TARGET test_${name} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:test_${name}> $<TARGET_RUNTIME_DLLS:test_${name}>
      COMMAND_EXPAND_LISTS)
  endif()
  add_test(NAME ${name} COMMAND test_${name} ${PPPP_TEST_ARGS})
  set_tests_properties(${name} PROPERTIES LABELS ${label} TIMEOUT 300)
endfunction()

if(PPPP_WITH_FOSU)
  pppp_add_test(decode fosu pppp_fosu fosu)
  pppp_add_test(difficulty fosu pppp_fosu)
  pppp_add_test(performance fosu pppp_fosu)
endif()

if(PPPP_WITH_FOSU AND PPPP_BUILD_CAPI)
  # --success names every test case as it runs, so a case that hangs and is killed by the timeout
  # above can be told apart from the ones before it in the CI log.
  set(PPPP_TEST_ARGS --success)
  pppp_add_test(capi bindings pppp_fosu pppp_c)
  unset(PPPP_TEST_ARGS)
endif()
