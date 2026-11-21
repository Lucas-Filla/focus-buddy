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

add_executable(focus_buddy src/main.cpp src/app/focus_rules.cpp)
target_link_libraries(focus_buddy PRIVATE raylib)

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