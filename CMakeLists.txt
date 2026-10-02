cmake_minimum_required(VERSION 3.24)
set(CMAKE_VERIFY_INTERFACE_HEADER_SETS ON)
project(pppp VERSION 0.0.0 LANGUAGES CXX
        DESCRIPTION "PerformancePoints++")

if(DEFINED SKBUILD)
  set(PPPP_BUILD_PYTHON ON)
  set(PPPP_INSTALL OFF)
endif()

if(DEFINED CMAKE_JS_VERSION)
  set(PPPP_BUILD_NODE ON)
  set(PPPP_INSTALL OFF)
  set(PPPP_BUILD_TESTS OFF)
endif()

option(PPPP_WITH_FOSU "Build the .osu parser" ON)
option(PPPP_BUILD_TESTS "Build the test suites" ${PROJECT_IS_TOP_LEVEL})
option(PPPP_INSTALL "Generate install rules" ${PROJECT_IS_TOP_LEVEL})
option(PPPP_BUILD_PYTHON "Build the Python extension module" OFF)
option(PPPP_BUILD_NODE "Build the Node bindings" OFF)

if(PPPP_BUILD_PYTHON OR PPPP_BUILD_NODE)
  # The library is static and the extension is a shared object.
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  # Nothing links LTO objects across the extension boundary.
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()

if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_CXX_STANDARD 98)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

include(GNUInstallDirs)
include(cmake/Warnings.cmake)
include(cmake/Numerics.cmake)
include(cmake/Performance.cmake)
include(cmake/Checks.cmake)

add_library(pppp_options INTERFACE)
target_compile_features(pppp_options INTERFACE cxx_std_98)
target_include_directories(pppp_options
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party/optional-bare/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if(PPPP_BUILD_TESTS)
  enable_testing()
endif()

add_library(pppp_utils
  src/pppp/utils/difficulty_calculation_utils.cpp
  src/pppp/utils/difficulty_range.cpp
  src/pppp/utils/math/csharp.cpp
  src/pppp/utils/math/legacy_score.cpp
  src/pppp/utils/math/uint128.cpp
  src/pppp/utils/random/csharp.cpp
  src/pppp/utils/random/osu.cpp
  src/pppp/utils/vector2.cpp)
add_library(pppp::utils ALIAS pppp_utils)
set_target_properties(pppp_utils PROPERTIES EXPORT_NAME utils)
target_include_directories(pppp_utils PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_utils PUBLIC pppp_options)
target_link_libraries(pppp_utils PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)
target_sources(pppp_utils PUBLIC FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES include/pppp/config.h include/pppp/utils/vector2.h)

add_library(pppp_beatmaps
  src/pppp/beatmaps/beatmap.cpp
  src/pppp/beatmaps/control_points/control_point_info.cpp
  src/pppp/beatmaps/slider_path.cpp)
add_library(pppp::beatmaps ALIAS pppp_beatmaps)
set_target_properties(pppp_beatmaps PROPERTIES EXPORT_NAME beatmaps)
target_include_directories(pppp_beatmaps PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_beatmaps PUBLIC pppp_utils)
target_link_libraries(pppp_beatmaps PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)
target_sources(pppp_beatmaps PUBLIC FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES include/pppp/beatmaps/beatmap.h include/pppp/beatmaps/timing/break_period.h
        include/pppp/beatmaps/slider_event_descriptor.h)

add_library(pppp_mods
  src/pppp/mods/mod.cpp
  src/pppp/mods/mod_legacy.cpp
  src/pppp/mods/mod_settings.cpp)
add_library(pppp::mods ALIAS pppp_mods)
set_target_properties(pppp_mods PROPERTIES EXPORT_NAME mods)
target_include_directories(pppp_mods PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_mods PUBLIC pppp_utils)
target_link_libraries(pppp_mods PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)
target_sources(pppp_mods PUBLIC FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES include/pppp/mods/mod.h)

add_library(pppp_common
  src/pppp/common/difficulty_attributes.cpp
  src/pppp/common/performance_attributes.cpp
  src/pppp/common/preprocessing/difficulty_hit_object.cpp
  src/pppp/common/skills/harmonic_skill.cpp
  src/pppp/common/skills/skill.cpp
  src/pppp/common/skills/strain_decay_skill.cpp
  src/pppp/common/skills/strain_skill.cpp
  src/pppp/common/skills/variable_length_strain_skill.cpp
  src/pppp/common/score_info.cpp)
add_library(pppp::common ALIAS pppp_common)
set_target_properties(pppp_common PROPERTIES EXPORT_NAME common)
target_include_directories(pppp_common PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_common PUBLIC pppp_utils pppp_mods)
target_link_libraries(pppp_common PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)
target_sources(pppp_common PUBLIC FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES include/pppp/common/difficulty_attributes.h
        include/pppp/common/performance_attributes.h
        include/pppp/common/hit_result.h
        include/pppp/common/score_info.h)

add_library(pppp_catch_base
  src/pppp/fruits/catch_beatmap.cpp
  src/pppp/fruits/catch_beatmap_converter.cpp
  src/pppp/fruits/catch_beatmap_processor.cpp
  src/pppp/fruits/mods/catch_mod_mirror.cpp
  src/pppp/fruits/object/banana_shower.cpp
  src/pppp/fruits/object/catch_hit_object.cpp
  src/pppp/fruits/object/juice_stream.cpp)
add_library(pppp::catch::base ALIAS pppp_catch_base)
set_target_properties(pppp_catch_base PROPERTIES EXPORT_NAME catch::base)
target_include_directories(pppp_catch_base PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_catch_base PUBLIC pppp_utils pppp_beatmaps pppp_common pppp_mods)
target_link_libraries(pppp_catch_base PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)

add_library(pppp_catch_difficulty
  src/pppp/fruits/difficulty/catch_difficulty_calculator.cpp
  src/pppp/fruits/difficulty/evaluators/movement_evaluator.cpp
  src/pppp/fruits/difficulty/preprocessing/catch_difficulty_hit_object.cpp
  src/pppp/fruits/difficulty/skills/movement.cpp)
add_library(pppp::catch::difficulty ALIAS pppp_catch_difficulty)
set_target_properties(pppp_catch_difficulty PROPERTIES EXPORT_NAME catch::difficulty)
target_include_directories(pppp_catch_difficulty PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_catch_difficulty PUBLIC pppp_catch_base pppp_common)
target_link_libraries(pppp_catch_difficulty PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)
target_sources(pppp_catch_difficulty PUBLIC FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES include/pppp/fruits/difficulty/catch_difficulty_attributes.h)

add_library(pppp_catch_performance
  src/pppp/fruits/difficulty/catch_performance_attributes.cpp
  src/pppp/fruits/difficulty/catch_performance_calculator.cpp
  )
add_library(pppp::catch::performance ALIAS pppp_catch_performance)
set_target_properties(pppp_catch_performance PROPERTIES EXPORT_NAME catch::performance)
target_include_directories(pppp_catch_performance PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_catch_performance PUBLIC pppp_catch_difficulty)
target_link_libraries(pppp_catch_performance PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)
target_sources(pppp_catch_performance PUBLIC FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES include/pppp/fruits/difficulty/catch_performance_attributes.h
        )

add_library(pppp_catch INTERFACE)
add_library(pppp::catch ALIAS pppp_catch)
set_target_properties(pppp_catch PROPERTIES EXPORT_NAME catch)
target_link_libraries(pppp_catch INTERFACE pppp_catch_performance)

add_library(pppp_mania_base
  src/pppp/mania/mods/mania_mod_mirror.cpp
  src/pppp/mania/mods/mania_mod_random.cpp
  src/pppp/mania/mods/mania_mod_hold_off.cpp
  src/pppp/mania/mods/mania_mod_invert.cpp
  src/pppp/mania/mania_beatmap.cpp
  src/pppp/mania/mania_beatmap_converter.cpp
  src/pppp/mania/patterns/hit_circle_pattern_generator.cpp
  src/pppp/mania/patterns/legacy_pattern_generator.cpp
  src/pppp/mania/patterns/pass_through_pattern_generator.cpp
  src/pppp/mania/patterns/slider_pattern_generator.cpp
  src/pppp/mania/patterns/spinner_pattern_generator.cpp)
add_library(pppp::mania::base ALIAS pppp_mania_base)
set_target_properties(pppp_mania_base PROPERTIES EXPORT_NAME mania::base)
target_include_directories(pppp_mania_base PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_mania_base PUBLIC pppp_utils pppp_beatmaps pppp_common pppp_mods)
target_link_libraries(pppp_mania_base PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)

add_library(pppp_mania_difficulty
  src/pppp/mania/difficulty/evaluators/individual_strain_evaluator.cpp
  src/pppp/mania/difficulty/evaluators/overall_strain_evaluator.cpp
  src/pppp/mania/difficulty/mania_difficulty_attributes.cpp
  src/pppp/mania/difficulty/mania_difficulty_calculator.cpp
  src/pppp/mania/difficulty/preprocessing/mania_difficulty_hit_object.cpp
  src/pppp/mania/difficulty/skills/strain.cpp)
add_library(pppp::mania::difficulty ALIAS pppp_mania_difficulty)
set_target_properties(pppp_mania_difficulty PROPERTIES EXPORT_NAME mania::difficulty)
target_include_directories(pppp_mania_difficulty PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_mania_difficulty PUBLIC pppp_mania_base pppp_common)
target_link_libraries(pppp_mania_difficulty PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)
target_sources(pppp_mania_difficulty PUBLIC FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES include/pppp/mania/difficulty/mania_difficulty_attributes.h)

add_library(pppp_mania_performance
  src/pppp/mania/difficulty/mania_performance_attributes.cpp
  src/pppp/mania/difficulty/mania_performance_calculator.cpp
  )
add_library(pppp::mania::performance ALIAS pppp_mania_performance)
set_target_properties(pppp_mania_performance PROPERTIES EXPORT_NAME mania::performance)
target_include_directories(pppp_mania_performance PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_mania_performance PUBLIC pppp_mania_difficulty)
target_link_libraries(pppp_mania_performance PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)
target_sources(pppp_mania_performance PUBLIC FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES include/pppp/mania/difficulty/mania_performance_attributes.h
        )

add_library(pppp_mania INTERFACE)
add_library(pppp::mania ALIAS pppp_mania)
set_target_properties(pppp_mania PROPERTIES EXPORT_NAME mania)
target_link_libraries(pppp_mania INTERFACE pppp_mania_performance)

add_library(pppp_osu_base
  src/pppp/osu/osu_beatmap.cpp
  src/pppp/osu/osu_beatmap_converter.cpp
  src/pppp/osu/osu_beatmap_processor.cpp
  src/pppp/osu/mods/osu_mod_hidden.cpp
  src/pppp/osu/mods/osu_mod_random.cpp
  src/pppp/osu/mods/osu_mod_target_practice.cpp
  src/pppp/osu/object/osu_hit_object.cpp
  src/pppp/osu/object/osu_hit_object_generation_utils.cpp
  src/pppp/osu/object/osu_playfield.cpp
  src/pppp/osu/osu_hit_windows.cpp)
add_library(pppp::osu::base ALIAS pppp_osu_base)
set_target_properties(pppp_osu_base PROPERTIES EXPORT_NAME osu::base)
target_include_directories(pppp_osu_base PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_osu_base PUBLIC pppp_utils pppp_beatmaps pppp_common pppp_mods)
target_link_libraries(pppp_osu_base PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)

add_library(pppp_osu_difficulty
  src/pppp/osu/difficulty/evaluators/aim/agility_evaluator.cpp
  src/pppp/osu/difficulty/evaluators/aim/flow_aim_evaluator.cpp
  src/pppp/osu/difficulty/evaluators/aim/snap_aim_evaluator.cpp
  src/pppp/osu/difficulty/evaluators/flashlight_evaluator.cpp
  src/pppp/osu/difficulty/evaluators/reading_evaluator.cpp
  src/pppp/osu/difficulty/evaluators/speed/rhythm_evaluator.cpp
  src/pppp/osu/difficulty/evaluators/speed/speed_evaluator.cpp
  src/pppp/osu/difficulty/legacy_score_simulator.cpp
  src/pppp/osu/difficulty/osu_difficulty_attributes.cpp
  src/pppp/osu/difficulty/osu_difficulty_calculator.cpp
  src/pppp/osu/difficulty/preprocessing/osu_difficulty_hit_object.cpp
  src/pppp/osu/difficulty/skills/aim.cpp
  src/pppp/osu/difficulty/skills/flashlight.cpp
  src/pppp/osu/difficulty/skills/reading.cpp
  src/pppp/osu/difficulty/skills/speed.cpp)
add_library(pppp::osu::difficulty ALIAS pppp_osu_difficulty)
set_target_properties(pppp_osu_difficulty PROPERTIES EXPORT_NAME osu::difficulty)
target_include_directories(pppp_osu_difficulty PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_osu_difficulty PUBLIC pppp_osu_base pppp_common)
target_link_libraries(pppp_osu_difficulty PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)
target_sources(pppp_osu_difficulty PUBLIC FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES include/pppp/osu/difficulty/osu_difficulty_attributes.h)

add_library(pppp_osu_performance
  src/pppp/osu/difficulty/osu_legacy_score_miss_calculator.cpp
  src/pppp/osu/difficulty/osu_performance_attributes.cpp
  src/pppp/osu/difficulty/osu_performance_calculator.cpp
  )
add_library(pppp::osu::performance ALIAS pppp_osu_performance)
set_target_properties(pppp_osu_performance PROPERTIES EXPORT_NAME osu::performance)
target_include_directories(pppp_osu_performance PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_osu_performance PUBLIC pppp_osu_difficulty)
target_link_libraries(pppp_osu_performance PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)
target_sources(pppp_osu_performance PUBLIC FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES include/pppp/osu/difficulty/osu_performance_attributes.h
        )

add_library(pppp_osu INTERFACE)
add_library(pppp::osu ALIAS pppp_osu)
set_target_properties(pppp_osu PROPERTIES EXPORT_NAME osu)
target_link_libraries(pppp_osu INTERFACE pppp_osu_performance)

add_library(pppp_taiko_base
  src/pppp/taiko/mods/taiko_mod_random.cpp
  src/pppp/taiko/mods/taiko_mod_swap.cpp
  src/pppp/taiko/taiko_beatmap.cpp
  src/pppp/taiko/taiko_beatmap_converter.cpp
  src/pppp/taiko/taiko_hit_windows.cpp)
add_library(pppp::taiko::base ALIAS pppp_taiko_base)
set_target_properties(pppp_taiko_base PROPERTIES EXPORT_NAME taiko::base)
target_include_directories(pppp_taiko_base PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_taiko_base PUBLIC pppp_utils pppp_beatmaps pppp_common pppp_mods)
target_link_libraries(pppp_taiko_base PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)

add_library(pppp_taiko_difficulty
  src/pppp/taiko/difficulty/evaluators/colour_evaluator.cpp
  src/pppp/taiko/difficulty/evaluators/reading_evaluator.cpp
  src/pppp/taiko/difficulty/evaluators/rhythm_evaluator.cpp
  src/pppp/taiko/difficulty/evaluators/stamina_evaluator.cpp
  src/pppp/taiko/difficulty/preprocessing/colour/taiko_colour_difficulty_preprocessor.cpp
  src/pppp/taiko/difficulty/preprocessing/rhythm/taiko_rhythm_difficulty_preprocessor.cpp
  src/pppp/taiko/difficulty/preprocessing/taiko_difficulty_hit_object.cpp
  src/pppp/taiko/difficulty/skills/colour.cpp
  src/pppp/taiko/difficulty/skills/reading.cpp
  src/pppp/taiko/difficulty/skills/rhythm.cpp
  src/pppp/taiko/difficulty/skills/stamina.cpp
  src/pppp/taiko/difficulty/taiko_difficulty_attributes.cpp
  src/pppp/taiko/difficulty/taiko_difficulty_calculator.cpp
  src/pppp/taiko/difficulty/utils/delta_time_normaliser.cpp
  src/pppp/taiko/difficulty/utils/interval_grouping_utils.cpp)
add_library(pppp::taiko::difficulty ALIAS pppp_taiko_difficulty)
set_target_properties(pppp_taiko_difficulty PROPERTIES EXPORT_NAME taiko::difficulty)
target_include_directories(pppp_taiko_difficulty PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_taiko_difficulty PUBLIC pppp_taiko_base pppp_common)
target_link_libraries(pppp_taiko_difficulty PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)
target_sources(pppp_taiko_difficulty PUBLIC FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES include/pppp/taiko/difficulty/taiko_difficulty_attributes.h)

add_library(pppp_taiko_performance
  src/pppp/taiko/difficulty/taiko_performance_attributes.cpp
  src/pppp/taiko/difficulty/taiko_performance_calculator.cpp
  )
add_library(pppp::taiko::performance ALIAS pppp_taiko_performance)
set_target_properties(pppp_taiko_performance PROPERTIES EXPORT_NAME taiko::performance)
target_include_directories(pppp_taiko_performance PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_taiko_performance PUBLIC pppp_taiko_difficulty)
target_link_libraries(pppp_taiko_performance PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)
target_sources(pppp_taiko_performance PUBLIC FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES include/pppp/taiko/difficulty/taiko_performance_attributes.h
        )

add_library(pppp_taiko INTERFACE)
add_library(pppp::taiko ALIAS pppp_taiko)
set_target_properties(pppp_taiko PROPERTIES EXPORT_NAME taiko)
target_link_libraries(pppp_taiko INTERFACE pppp_taiko_performance)

add_library(pppp_api
  src/pppp/difficulty.cpp
  src/pppp/performance.cpp)
add_library(pppp::api ALIAS pppp_api)
set_target_properties(pppp_api PROPERTIES EXPORT_NAME api)
target_include_directories(pppp_api PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_api PUBLIC pppp_catch pppp_mania pppp_osu pppp_taiko)
target_link_libraries(pppp_api PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)
target_sources(pppp_api PUBLIC FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES include/pppp/pppp.h include/pppp/difficulty.h include/pppp/performance.h)

add_library(pppp INTERFACE)
add_library(pppp::pppp ALIAS pppp)
set_target_properties(pppp PROPERTIES EXPORT_NAME pppp)
target_link_libraries(pppp INTERFACE pppp_api)

# optional
if(PPPP_INSTALL)
  install(DIRECTORY third_party/optional-bare/include/nonstd DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  install(FILES third_party/optional-bare/LICENSE.txt DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nonstd)
endif()

# fosu
if(PPPP_WITH_FOSU)
  set(FOSU_BUILD_TESTS ${PPPP_BUILD_TESTS})
  set(FOSU_WARNINGS_AS_ERRORS ${PPPP_WARNINGS_AS_ERRORS})

  add_subdirectory(third_party/fosu)

  set_target_properties(fosu PROPERTIES EXPORT_NAME fosu_parser)

  add_library(pppp_fosu src/pppp/fosu/beatmap.cpp)
  add_library(pppp::fosu ALIAS pppp_fosu)

  target_compile_features(pppp_fosu PUBLIC cxx_std_98)
  target_include_directories(pppp_fosu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
  target_link_libraries(pppp_fosu PUBLIC pppp fosu)
  target_link_libraries(pppp_fosu PRIVATE $<BUILD_INTERFACE:pppp_warnings> $<BUILD_INTERFACE:pppp_numerics> $<BUILD_INTERFACE:pppp_performance>)
  set_target_properties(pppp_fosu PROPERTIES EXPORT_NAME fosu VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
  target_sources(pppp_fosu PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES include/pppp/beatmap.h)
endif()

add_library(pppp_internal INTERFACE)
target_include_directories(pppp_internal INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_internal INTERFACE pppp)

if(PPPP_BUILD_PYTHON)
  include(cmake/Python.cmake)
endif()

if(PPPP_BUILD_NODE)
  include(cmake/Node.cmake)
endif()

if(PPPP_BUILD_TESTS)
  add_subdirectory(tests)
endif()

if(PPPP_INSTALL)
  include(CMakePackageConfigHelpers)

  set(PPPP_INSTALL_TARGETS pppp pppp_options pppp_api
    pppp_utils pppp_beatmaps pppp_mods pppp_common
    pppp_catch_base pppp_catch_difficulty pppp_catch_performance pppp_catch
    pppp_mania_base pppp_mania_difficulty pppp_mania_performance pppp_mania
    pppp_osu_base pppp_osu_difficulty pppp_osu_performance pppp_osu
    pppp_taiko_base pppp_taiko_difficulty pppp_taiko_performance pppp_taiko)
  if(PPPP_WITH_FOSU)
    list(APPEND PPPP_INSTALL_TARGETS pppp_fosu fosu)
  endif()

  install(TARGETS ${PPPP_INSTALL_TARGETS} EXPORT ppppTargets
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  if(PPPP_WITH_FOSU)
    install(DIRECTORY third_party/fosu/src/fosu DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      FILES_MATCHING PATTERN "*.h")
  endif()
  install(EXPORT ppppTargets NAMESPACE pppp:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pppp)

  configure_package_config_file(cmake/ppppConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ppppConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pppp)
  write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/ppppConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ppppConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ppppConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pppp)
endif()
