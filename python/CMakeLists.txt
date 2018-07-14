find_program(PYTHON "${PYTHON_VERSION}")

function(python_install SETUP_PY)
    if(EXISTS "$ENV{DESTDIR}/${CMAKE_INSTALL_PREFIX}/lib/${PYTHON_VERSION}/dist-packages")
        install(CODE "
        execute_process(
            COMMAND ${PYTHON} ${SETUP_PY} install
                --record python_manifest.txt
                --root=/\$ENV{DESTDIR}
                --install-lib=${CMAKE_INSTALL_PREFIX}/lib/${PYTHON_VERSION}/dist-packages
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )")
    elseif(DEFINED "$ENV{CUSTOM_PREFIX}")
        install(CODE "
        execute_process(
            COMMAND ${PYTHON} ${SETUP_PY} install
                --record python_manifest.txt
                --root=/\$ENV{DESTDIR}
                --prefix=${CMAKE_INSTALL_PREFIX}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )")
    else()
        install(CODE "
        execute_process(
            COMMAND ${PYTHON} ${SETUP_PY} install
                --record python_manifest.txt
                --root=/\$ENV{DESTDIR}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )")
    endif()
endfunction()

set(SETUP_PY ${CMAKE_SOURCE_DIR}/python/setup.py)
set(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/timestamp)
set(DEPS
        ${CMAKE_SOURCE_DIR}/python/adcusb.pyx
        ${CMAKE_SOURCE_DIR}/python/adcusb.pxd
        ${CMAKE_SOURCE_DIR}/python/setup.py)

add_custom_command(
        OUTPUT ${OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/python
            ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E env
            CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}
            CMAKE_PREFIX=${CMAKE_INSTALL_PREFIX}
            ${PYTHON} ${SETUP_PY} build
        COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT}
        DEPENDS ${DEPS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(py-adcusb ALL DEPENDS ${OUTPUT})

python_install(${SETUP_PY})
