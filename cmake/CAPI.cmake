include(CheckCSourceCompiles)

add_library(pppp_c SHARED src/pppp/bindings/capi.cpp)
set_target_properties(pppp_c PROPERTIES
  PREFIX ""
  CXX_VISIBILITY_PRESET hidden)
target_include_directories(pppp_c PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(pppp_c PRIVATE PPPP_C_BUILD)
target_link_libraries(pppp_c PRIVATE
  pppp_fosu
  $<BUILD_INTERFACE:pppp_warnings>
  $<BUILD_INTERFACE:pppp_numerics>
  $<BUILD_INTERFACE:pppp_performance>
  $<BUILD_INTERFACE:pppp_bindings>)
target_sources(pppp_c PUBLIC FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES include/pppp/capi.h)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(pppp_c PRIVATE "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/capi.map")
  set_property(TARGET pppp_c APPEND PROPERTY LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/capi.map")
endif()

if(PPPP_WIN2K)
  target_sources(pppp_c PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/win2k/get-thread-id.c)
endif()

set(CMAKE_REQUIRED_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/include")
check_c_source_compiles(
  "#include <pppp/capi.h>
   int main(void) {
       pppp_beatmap* map = 0;
       pppp_difficulty_options options = {0};
       return map == 0 && options.has_ruleset == 0;
   }"
  PPPP_CAPI_HEADER_IS_C)
if(NOT PPPP_CAPI_HEADER_IS_C)
  message(FATAL_ERROR "include/pppp/capi.h must compile as C")
endif()

if(TARGET fosu_engine_avx2)
  add_custom_command(TARGET pppp_c POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:fosu_engine_avx2> $<TARGET_FILE_DIR:pppp_c>)
elseif(TARGET fosu_engine_neon)
  add_custom_command(TARGET pppp_c POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:fosu_engine_neon> $<TARGET_FILE_DIR:pppp_c>)
endif()
