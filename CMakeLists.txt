cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(test_leptjson LANGUAGES C CXX)

# gtest configuration
set(CMAKE_CXX_STANDARD 14)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_subdirectory(src)

add_executable(
  test_leptjson
  test/test_leptjson.cc
)

target_link_libraries(
  test_leptjson
  GTest::gtest_main
  leptjson
)

add_executable(
  test_addtion
  test/test_addtion.c
)

target_link_libraries(
    test_addtion
    leptjson
)


include(GoogleTest)
gtest_discover_tests(test_leptjson)