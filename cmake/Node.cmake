add_library(pppp_node SHARED src/pppp/bindings/node.cpp ${CMAKE_JS_SRC})
set_target_properties(pppp_node PROPERTIES
  PREFIX ""
  SUFFIX ".node"
  INTERPROCEDURAL_OPTIMIZATION OFF)
target_compile_features(pppp_node PRIVATE cxx_std_11)
target_compile_definitions(pppp_node PRIVATE NAPI_VERSION=8)
target_include_directories(pppp_node PRIVATE ${CMAKE_JS_INC} ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pppp_node PRIVATE ${CMAKE_JS_LIB} pppp_fosu)

if(MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
  execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS})
endif()
