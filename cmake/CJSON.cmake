include(FetchContent)

enable_language(C)

FetchContent_Declare(cjson
  URL https://github.com/DaveGamble/cJSON/archive/c859b25da02955fef659d658b8f324b5cde87be3.tar.gz
  URL_HASH SHA256=880a86445ad5cf38f877bac31198268d8fac5d2ce996ebf55d3066a4109ddd8d
  SOURCE_SUBDIR library_config)
FetchContent_MakeAvailable(cjson)

add_library(pppp_cjson STATIC ${cjson_SOURCE_DIR}/cJSON.c)
target_include_directories(pppp_cjson SYSTEM PUBLIC ${cjson_SOURCE_DIR})
