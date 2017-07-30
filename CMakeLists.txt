cmake_minimum_required(VERSION 3.8)
project(libadcusb)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-fblocks -Wall -Wextra")

find_package (PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

set(SOURCE_FILES src/adcusb.c src/adcusb.h)

include_directories(${LIBUSB_INCLUDE_DIRS})
link_directories(${LIBUSB_LIBRARY_DIRS})
include_directories (${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})

add_library(libadcusb SHARED ${SOURCE_FILES})
target_link_libraries(libadcusb ${GLIB_LIBRARIES})
target_link_libraries(libadcusb ${LIBUSB_LIBRARIES})

if(NOT APPLE)
    target_link_libraries(libadcusb BlocksRuntime)
endif()

add_subdirectory(examples/read)
