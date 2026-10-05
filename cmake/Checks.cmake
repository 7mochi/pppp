if(NOT PROJECT_IS_TOP_LEVEL)
  return()
endif()

find_program(CLANG_FORMAT NAMES clang-format)

file(GLOB_RECURSE PPPP_CODESTYLE_HEADERS CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h)
file(GLOB_RECURSE PPPP_CODESTYLE_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)

if(CLANG_FORMAT)
  add_custom_target(format
    COMMAND ${CLANG_FORMAT} -i ${PPPP_CODESTYLE_HEADERS} ${PPPP_CODESTYLE_SOURCES}
    COMMENT "Formatting the pppp sources")
  add_custom_target(check_format
    COMMAND ${CLANG_FORMAT} --dry-run --Werror ${PPPP_CODESTYLE_HEADERS} ${PPPP_CODESTYLE_SOURCES}
    COMMENT "Checking the format of the pppp sources")
endif()
