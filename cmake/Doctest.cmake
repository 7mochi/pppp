include(FetchContent)

if(APPLE)
  set(PPPP_DOCTEST_PATCH
    PATCH_COMMAND patch -p1 -i ${CMAKE_CURRENT_LIST_DIR}/../tests/patches/doctest-arm64.patch)
endif()

FetchContent_Declare(doctest
  URL https://github.com/doctest/doctest/archive/cd1d7474374c8cebed5c609564cdaa2f5184be25.tar.gz
  URL_HASH SHA256=cd9d23bba8247dfb47b132d7fb16414db72c451b1c51dd15d332dd064424250f
  SOURCE_SUBDIR doctest
  ${PPPP_DOCTEST_PATCH})
FetchContent_MakeAvailable(doctest)

add_library(pppp_doctest INTERFACE)
target_include_directories(pppp_doctest SYSTEM INTERFACE ${doctest_SOURCE_DIR}/doctest)
target_compile_definitions(pppp_doctest INTERFACE DOCTEST_CONFIG_NO_POSIX_SIGNALS)

add_library(pppp_test_main OBJECT ${CMAKE_CURRENT_LIST_DIR}/../tests/main.cpp)
target_compile_features(pppp_test_main PRIVATE cxx_std_98)
target_link_libraries(pppp_test_main PUBLIC pppp_doctest)
