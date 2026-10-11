cmake_minimum_required(VERSION 3.21)
project(fosu VERSION 0.8.0 LANGUAGES CXX)

option(FOSU_BUILD_TESTS "Build the fosu test suites" ${PROJECT_IS_TOP_LEVEL})
option(FOSU_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
set(FOSU_ISA auto CACHE STRING "Native targets: auto, scalar, avx2, neon")
set_property(CACHE FOSU_ISA PROPERTY STRINGS auto scalar avx2 neon)

if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_CXX_STANDARD 98)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

include(GNUInstallDirs)

# One module defines one engine: the scalar core dispatches at runtime, the engine libraries pick
# their instructions at build time. FOSU_ISA selects which engine libraries are built.
set(_fosu_processor "${CMAKE_SYSTEM_PROCESSOR}")
if(APPLE AND CMAKE_OSX_ARCHITECTURES)
  set(_fosu_processor "${CMAKE_OSX_ARCHITECTURES}")
endif()
set(_fosu_x86 OFF)
if(_fosu_processor MATCHES "^(x86_64|AMD64|amd64)$")
  set(_fosu_x86 ON)
endif()
set(_fosu_arm64 OFF)
if(_fosu_processor MATCHES "^(aarch64|arm64|ARM64)$")
  set(_fosu_arm64 ON)
endif()
if(NOT FOSU_ISA MATCHES "^(auto|scalar|avx2|neon)$")
  message(FATAL_ERROR "FOSU_ISA must be auto, scalar, avx2 or neon")
endif()
if(FOSU_ISA STREQUAL "avx2" AND NOT _fosu_x86)
  message(FATAL_ERROR "AVX2 requires an x86-64 target; use a cross-compilation toolchain")
endif()
if(FOSU_ISA STREQUAL "neon" AND NOT _fosu_arm64)
  message(FATAL_ERROR "NEON requires an AArch64 target")
endif()

# Additive to the project's C++98 flags: one ISA's definitions and instruction selection for one
# compiled product (`scalar` embeds no SIMD, `avx2` selects it at build time).
function(fosu_configure target isa product)
  if(isa STREQUAL "scalar")
    target_compile_definitions(${target} PRIVATE FOSU_DISABLE_SIMD)
  elseif(isa STREQUAL "avx2")
    target_compile_definitions(${target} PRIVATE FOSU_COMPILE_AVX2)
  endif()
  if(MSVC)
    target_compile_options(${target} PRIVATE /fp:precise)
  else()
    target_compile_options(${target} PRIVATE -ffp-contract=off)
  endif()
  if(_fosu_x86 AND isa STREQUAL "scalar" AND NOT MSVC)
    target_compile_options(${target} PRIVATE -march=x86-64)
  endif()
  if(product STREQUAL "python")
    if(isa STREQUAL "avx2")
      if(MSVC)
        target_compile_options(${target} PRIVATE /arch:AVX2)
      else()
        target_compile_options(${target} PRIVATE -mavx2 -mbmi -mbmi2)
      endif()
    endif()
  elseif(isa STREQUAL "avx2")
    if(MSVC)
      target_compile_options(${target} PRIVATE /arch:AVX2)
    else()
      target_compile_options(${target} PRIVATE -march=x86-64-v3)
    endif()
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(isa STREQUAL "avx2" OR product STREQUAL "python")
      target_compile_options(${target} PRIVATE -fno-plt)
    endif()
  endif()
endfunction()

function(fosu_runtime target)
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(${target} PRIVATE -Wl,--exclude-libs,ALL)
  endif()
endfunction()

# The core owns Parser lifetime and storage. The ISA libraries execute the shared parser, so the
# core is compiled scalar and is the never-null fallback the loader resolves to.
add_library(fosu STATIC
  src/fosu/engine/parsing/decimal.cpp
  src/fosu/engine/parsing/lookup_tables.cpp
  src/fosu/engine/parse_document.cpp
  src/fosu/engine/runtime/loader.cc
  src/fosu/runtime.cc)
target_compile_features(fosu PUBLIC cxx_std_98)
target_compile_definitions(fosu PRIVATE
  FOSU_DEFAULT_BACKEND="${FOSU_ISA}"
  FOSU_ENGINE_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}"
  FOSU_BUILDING_RUNTIME)
target_link_libraries(fosu PRIVATE ${CMAKE_DL_LIBS})
target_include_directories(fosu
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../optional-bare/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../span-lite/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../string-view-lite/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../atomic-lite/include>
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

fosu_configure(fosu scalar native)

# An engine library per applicable ISA. It has to be self-contained: the port's tables and decimal
# conversion are out-of-line TUs, and the core cannot be linked in (the core depends on the engine
# to build it, so a link dependency would be a cycle). Hidden visibility keeps the engine's
# `compiled_engine` from being preempted by the core's scalar copy.
function(fosu_dispatch target)
  set(variant "")
  if(_fosu_x86 AND NOT FOSU_ISA STREQUAL "scalar")
    set(variant avx2)
    target_compile_definitions(${target} PRIVATE FOSU_HAS_AVX2)
  elseif(_fosu_arm64 AND NOT FOSU_ISA STREQUAL "scalar")
    set(variant neon)
    target_compile_definitions(${target} PRIVATE FOSU_HAS_NEON)
  endif()
  if(variant)
    set(engine fosu_engine_${variant})
    if(NOT TARGET ${engine})
      add_library(${engine} SHARED
        src/fosu/engine/runtime/entry.cc
        src/fosu/engine/parse_document.cpp
        src/fosu/engine/parsing/decimal.cpp
        src/fosu/engine/parsing/lookup_tables.cpp)
      target_compile_features(${engine} PRIVATE cxx_std_98)
      target_include_directories(${engine} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../optional-bare/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../span-lite/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../string-view-lite/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../atomic-lite/include)
      target_link_libraries(${engine} PRIVATE $<BUILD_INTERFACE:fosu_warnings>)
      fosu_configure(${engine} "${variant}" native)
      fosu_runtime(${engine})
      # R10: the engine lands in a subdirectory, so a ctest binary's own dlopen of the sibling
      # engine cannot find it; the dispatch suite expects the embedded scalar fallback.
      set_target_properties(${engine} PROPERTIES
        OUTPUT_NAME fosu_engine_${variant}
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/engine"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/engine")
    endif()
    add_dependencies(${target} ${engine})
  endif()
endfunction()

fosu_dispatch(fosu)

if(FOSU_BUILD_TESTS)
  if(NOT TARGET pppp_test_main)
    message(FATAL_ERROR
      "The fosu suites run on the project's doctest, which pppp brings up in cmake/Doctest.cmake "
      "before adding this directory. Configure the project instead of this directory, or build the "
      "parser alone with -DFOSU_BUILD_TESTS=OFF.")
  endif()
  enable_testing()
  find_package(Threads REQUIRED)

  function(fosu_add_test name)
    add_executable(fosu_test_${name} tests/test_${name}.cpp)
    target_compile_features(fosu_test_${name} PRIVATE cxx_std_98)
    fosu_configure(fosu_test_${name} scalar native)
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
  fosu_add_test(dispatch)
  target_link_libraries(fosu_test_dispatch PRIVATE Threads::Threads)
endif()
