if(CMAKE_VERSION VERSION_LESS 3.26)
  message(FATAL_ERROR
    "The Python extension needs CMake 3.26 for Development.SABIModule and "
    "Python_add_library(USE_SABI); found ${CMAKE_VERSION}")
endif()

if(WIN32)
  find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
  list(GET Python_LIBRARIES 0 _python_library)
  get_filename_component(_python_library_directory "${_python_library}" DIRECTORY)
  set(_python_sabi_library "${_python_library_directory}/python3.lib")
  if(NOT EXISTS "${_python_sabi_library}")
    message(FATAL_ERROR "python3.lib was not found beside the Windows Python installation")
  endif()
  add_library(Python::SABIModule INTERFACE IMPORTED)
  set_target_properties(Python::SABIModule PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${Python_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${_python_sabi_library}")
else()
  find_package(Python REQUIRED COMPONENTS Interpreter Development.SABIModule)
endif()

Python_add_library(_core MODULE USE_SABI 3.10 WITH_SOABI
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pppp/bindings/python.cpp)
set_target_properties(_core PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  INTERPROCEDURAL_OPTIMIZATION OFF)
target_include_directories(_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(_core PRIVATE
  pppp_fosu
  $<BUILD_INTERFACE:pppp_warnings>
  $<BUILD_INTERFACE:pppp_numerics>
  $<BUILD_INTERFACE:pppp_performance>)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(_core PRIVATE
    "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/python/core.map")
  set_property(TARGET _core APPEND PROPERTY LINK_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/python/core.map")
elseif(APPLE)
  target_link_options(_core PRIVATE "-Wl,-exported_symbol,_PyInit__core")
endif()

install(TARGETS _core LIBRARY DESTINATION pppp RUNTIME DESTINATION pppp)
