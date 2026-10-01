if(NOT PROJECT_IS_TOP_LEVEL)
  return()
endif()

find_program(CLANG_FORMAT NAMES clang-format)
find_program(CLANG_TIDY NAMES clang-tidy)

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
  add_custom_target(format-check
    COMMAND ${CLANG_FORMAT} --dry-run --Werror ${PPPP_CODESTYLE_HEADERS} ${PPPP_CODESTYLE_SOURCES}
    COMMENT "Checking the format of the pppp sources")
endif()

if(CLANG_TIDY)
  add_custom_target(check-tidy
    COMMAND ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR} --quiet ${PPPP_CODESTYLE_SOURCES})
endif()
