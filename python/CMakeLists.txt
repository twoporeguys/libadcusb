find_program(PYTHON "python${PYTHON_VERSION}")

set(SETUP_PY ${CMAKE_SOURCE_DIR}/python/setup.py)
set(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/timestamp)
set(DEPS
        ${CMAKE_SOURCE_DIR}/python/adcusb.pyx
        ${CMAKE_SOURCE_DIR}/python/adcusb.pxd)

add_custom_command(OUTPUT ${OUTPUT}
        COMMAND cp -a ${CMAKE_SOURCE_DIR}/python/* ${CMAKE_CURRENT_BINARY_DIR}/
        COMMAND env LDFLAGS="-L${CMAKE_CURRENT_BINARY_DIR}/.." CFLAGS="-I${CMAKE_SOURCE_DIR}/include" ${PYTHON} ${SETUP_PY} build
        COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT}
        DEPENDS ${DEPS}
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(py-adcusb ALL DEPENDS ${OUTPUT})

install(CODE "execute_process(COMMAND ${PYTHON} ${SETUP_PY} install --root=\$DESTDIR WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})")