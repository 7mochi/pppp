include(CheckCSourceCompiles)

add_library(pppp_c SHARED src/pppp/bindings/capi.cpp)
set_target_properties(pppp_c PROPERTIES
  PREFIX ""
  CXX_VISIBILITY_PRESET hidden
  INTERPROCEDURAL_OPTIMIZATION OFF)
target_include_directories(pppp_c PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(pppp_c PRIVATE PPPP_C_BUILD)
target_link_libraries(pppp_c PRIVATE
  pppp_fosu
  $<BUILD_INTERFACE:pppp_warnings>
  $<BUILD_INTERFACE:pppp_numerics>
  $<BUILD_INTERFACE:pppp_performance>)
target_sources(pppp_c PUBLIC FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES include/pppp/capi.h)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(pppp_c PRIVATE "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/capi.map")
  set_property(TARGET pppp_c APPEND PROPERTY LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/capi.map")
endif()

enable_language(C)
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
