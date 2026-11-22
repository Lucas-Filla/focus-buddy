cmake_minimum_required(VERSION 3.24)
project(FocusBuddy LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

add_library(focus_core STATIC src/app/focus_rules.cpp)
target_include_directories(focus_core PUBLIC src)

add_executable(focus_buddy src/main.cpp)
target_link_libraries(focus_buddy PRIVATE focus_core raylib)

if(WIN32)
  target_sources(focus_buddy PRIVATE 
    src/platform/win32/overlay_win32.cpp
    src/platform/win32/focus_monitor_win32.cpp)
endif()

add_custom_target(copy_assets ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/assets
          $<TARGET_FILE_DIR:focus_buddy>/assets)
add_dependencies(focus_buddy copy_assets)

enable_testing()

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    GIT_SHALLOW TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(focus_tests tests/focus_rules_test.cpp)
target_link_libraries(focus_tests PRIVATE focus_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(focus_tests)