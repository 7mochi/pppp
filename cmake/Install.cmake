include(CMakePackageConfigHelpers)

# nonstd
install(DIRECTORY third_party/optional-bare/include/nonstd DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY third_party/variant-lite/include/nonstd DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES third_party/optional-bare/LICENSE.txt
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nonstd RENAME optional-bare-LICENSE.txt)
install(FILES third_party/variant-lite/LICENSE.txt
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nonstd RENAME variant-lite-LICENSE.txt)
if(PPPP_WITH_FOSU)
  install(DIRECTORY third_party/span-lite/include/nonstd DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  install(DIRECTORY third_party/string-view-lite/include/nonstd DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  install(DIRECTORY third_party/atomic-lite/include/nonstd DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  install(FILES third_party/span-lite/LICENSE.txt
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nonstd RENAME span-lite-LICENSE.txt)
  install(FILES third_party/string-view-lite/LICENSE.txt
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nonstd RENAME string-view-lite-LICENSE.txt)
  install(FILES third_party/atomic-lite/LICENSE.txt
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nonstd RENAME atomic-lite-LICENSE.txt)
endif()

set(PPPP_INSTALL_TARGETS pppp pppp_options pppp_api
  pppp_utils pppp_beatmaps pppp_mods pppp_common
  pppp_catch_base pppp_catch_difficulty pppp_catch_performance pppp_catch
  pppp_mania_base pppp_mania_difficulty pppp_mania_performance pppp_mania
  pppp_osu_base pppp_osu_difficulty pppp_osu_performance pppp_osu
  pppp_taiko_base pppp_taiko_difficulty pppp_taiko_performance pppp_taiko)
if(PPPP_WITH_FOSU)
  list(APPEND PPPP_INSTALL_TARGETS pppp_fosu fosu)
  if(TARGET fosu_engine_avx2)
    list(APPEND PPPP_INSTALL_TARGETS fosu_engine_avx2)
  elseif(TARGET fosu_engine_neon)
    list(APPEND PPPP_INSTALL_TARGETS fosu_engine_neon)
  endif()
  if(PPPP_BUILD_CAPI)
    list(APPEND PPPP_INSTALL_TARGETS pppp_c)
  endif()
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
  cmake/ppppPlaceEngine.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pppp)
