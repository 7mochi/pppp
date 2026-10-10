cmake_minimum_required(VERSION 3.21)
project(fosu VERSION 0.8.0 LANGUAGES CXX)

option(FOSU_BUILD_TESTS "Build the fosu test suites" ${PROJECT_IS_TOP_LEVEL})
option(FOSU_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_CXX_STANDARD 98)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

include(GNUInstallDirs)

add_library(fosu STATIC
  src/fosu/engine/parsing/decimal.cpp
  src/fosu/engine/parsing/lookup_tables.cpp)
target_compile_features(fosu PUBLIC cxx_std_98)
target_include_directories(fosu
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../optional-bare/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../span-lite/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../string-view-lite/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set_target_properties(fosu PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>")

add_library(fosu_warnings INTERFACE)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(fosu_warnings INTERFACE
    -Wall -Wextra -Wpedantic -Wshadow -Wundef -Wcast-align -Wno-long-long)
  if(FOSU_WARNINGS_AS_ERRORS)
    target_compile_options(fosu_warnings INTERFACE -Werror)
  endif()
  target_compile_options(fosu PUBLIC -ffp-contract=off)
  if(CMAKE_SIZEOF_VOID_P EQUAL 4 AND CMAKE_SYSTEM_PROCESSOR MATCHES "i[3-6]86|x86")
    target_compile_options(fosu PUBLIC -msse2 -mfpmath=sse)
  endif()
elseif(MSVC)
  target_compile_options(fosu_warnings INTERFACE /W4)
  if(FOSU_WARNINGS_AS_ERRORS)
    target_compile_options(fosu_warnings INTERFACE /WX)
  endif()
  target_compile_options(fosu PUBLIC /fp:precise)
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_compile_options(fosu PUBLIC /arch:SSE2)
  endif()
endif()
target_link_libraries(fosu PRIVATE $<BUILD_INTERFACE:fosu_warnings>)
set_target_properties(fosu PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

if(FOSU_BUILD_TESTS)
  if(NOT TARGET pppp_test_main)
    message(FATAL_ERROR
      "The fosu suites run on the project's doctest, which pppp brings up in cmake/Doctest.cmake "
      "before adding this directory. Configure the project instead of this directory, or build the "
      "parser alone with -DFOSU_BUILD_TESTS=OFF.")
  endif()
  enable_testing()

  function(fosu_add_test name)
    add_executable(fosu_test_${name} tests/test_${name}.cpp)
    target_compile_features(fosu_test_${name} PRIVATE cxx_std_98)
    target_link_libraries(fosu_test_${name} PRIVATE fosu fosu_warnings pppp_doctest pppp_test_main)
    add_test(NAME fosu_${name} COMMAND fosu_test_${name})
    set_tests_properties(fosu_${name} PROPERTIES LABELS "parser" TIMEOUT 300)
  endfunction()
  fosu_add_test(decimal)
  fosu_add_test(sections)
  fosu_add_test(hitobject_fields)
  fosu_add_test(storage)
  fosu_add_test(hardening)
  fosu_add_test(numeric)
  fosu_add_test(string_lookup)
  fosu_add_test(mods)
  fosu_add_test(stacking)
  fosu_add_test(slider_paths)
  fosu_add_test(slider_timing)
  fosu_add_test(clients)
endif()
