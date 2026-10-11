# Copy the parser's SIMD engine next to a consumer target. The parser's core is static, so its loader
# searches for `fosu_engine_*` beside the consumer's own binary; call this after `target_link_libraries`
# with `pppp::fosu`. It is a no-op on architectures that build no engine (the library uses scalar).
function(pppp_place_engine target)
  if(NOT TARGET ${target})
    message(FATAL_ERROR "pppp_place_engine: '${target}' is not a target")
  endif()
  foreach(variant avx2 neon)
    set(engine "pppp::fosu_engine_${variant}")
    if(TARGET ${engine})
      add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:${engine}> $<TARGET_FILE_DIR:${target}>)
      return()
    endif()
  endforeach()
endfunction()
