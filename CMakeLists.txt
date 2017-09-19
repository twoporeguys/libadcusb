cmake_minimum_required(VERSION 3.2)
project(libadcusb)

set(PYTHON_VERSION 3)
option(BUNDLED_BLOCKS_RUNTIME "Use bundled version of BlocksRuntime")

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-fblocks -Wall -Wextra")

find_package (PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

set(HEADERS include/adcusb.h)
set(SOURCE_FILES src/adcusb.c)

if(BUNDLED_BLOCKS_RUNTIME)
    include_directories(contrib/BlocksRuntime)
    set(SOURCE_FILES
            ${SOURCE_FILES}
            contrib/BlocksRuntime/data.c
            contrib/BlocksRuntime/runtime.c)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${LIBUSB_INCLUDE_DIRS})
link_directories(${LIBUSB_LIBRARY_DIRS})
include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})

add_library(libadcusb SHARED ${SOURCE_FILES} ${HEADERS})
set_target_properties(libadcusb PROPERTIES PREFIX "")
target_link_libraries(libadcusb ${GLIB_LIBRARIES})
target_link_libraries(libadcusb ${LIBUSB_LIBRARIES})

if(NOT APPLE)
    target_link_libraries(libadcusb BlocksRuntime)
endif()

add_subdirectory(python)
add_subdirectory(examples/read)
