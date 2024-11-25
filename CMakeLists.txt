cmake_minimum_required (VERSION 3.30)

project (touchBar)

# Find the DRM library
find_package(PkgConfig REQUIRED)
pkg_check_modules(DRM REQUIRED libdrm)

add_executable(touchBar main.c include/manager.h src/manager.c include/g_graphics.h src/g_graphics.c)

target_include_directories(touchBar PRIVATE ${DRM_INCLUDE_DIRS})
target_link_libraries(touchBar PRIVATE ${DRM_LIBRARIES})
