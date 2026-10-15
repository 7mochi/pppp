function(pppp_apply_defaults target export_name)
  set_target_properties(${target} PROPERTIES EXPORT_NAME ${export_name})
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
  target_link_libraries(${target} PRIVATE
    $<BUILD_INTERFACE:pppp_warnings>
    $<BUILD_INTERFACE:pppp_numerics>
    $<BUILD_INTERFACE:pppp_performance>)
endfunction()
