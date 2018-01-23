#
# Copyright 2017 Two Pore Guys, Inc.
# All rights reserved
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted providing that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

import os
import numpy as np
from contextlib import contextmanager
cimport numpy as np
from libc.stdlib cimport malloc, free
from libc.string cimport memcpy
from libc.stdint cimport *
from libc.errno cimport errno
from cpython cimport PyObject, Py_INCREF
from adcusb cimport *


np.import_array()


class ADCException(RuntimeError):
    pass


cdef class ADCDataBlock(object):
    cdef adcusb_data_block *block
    cdef object ndarray

    def __dealloc__(self):
        free(<void *>self.block)

    cdef init(self):
        cdef np.npy_intp shape[1]
        shape[0] = <np.npy_intp>self.block.adb_count
        self.ndarray = np.PyArray_SimpleNewFromData(1, shape, np.NPY_INT32, self.block.adb_samples)
        Py_INCREF(self)
        PyArray_SetBaseObject(self.ndarray, self)

    property seqno:
        def __get__(self):
            return self.block.adb_seqno

    property length:
        def __get__(self):
            return self.block.adb_count

    property overrun:
        def __get__(self):
            return self.block.adb_overrun

    property samples:
        def __get__(self):
            return self.ndarray

cdef class ADC(object):
    cdef adcusb_device_t dev
    cdef public object callback

    def __init__(self, address=None, serial=None):
        cdef const char *c_serial
        cdef int c_address
        cdef int ret

        self.dev = <adcusb_device_t>NULL

        if not address and not serial:
            raise ADCException('Address or serial must be provided')

        if serial:
            u_serial = serial.encode('utf-8')
            c_serial = u_serial
            with nogil:
                ret = adcusb_open_by_serial(c_serial, &self.dev)

        elif address:
            c_address = address
            with nogil:
                ret = adcusb_open_by_address(c_address, &self.dev)

        if ret != 0:
            raise ADCException(os.strerror(errno))

        adcusb_set_callback(self.dev, ADCUSB_CALLBACK(self.c_callback, <void *>self))

    def __dealloc__(self):
        if self.dev != <adcusb_device_t>NULL:
            with nogil:
                adcusb_close(self.dev)

    def start(self):
        with nogil:
            adcusb_start(self.dev)

    def stop(self):
        with nogil:
            adcusb_stop(self.dev)

    def read(self):
        @contextmanager
        def read_wrapped():
            self.start()
            try:
                yield
            finally:
                self.stop()

        return read_wrapped()

    @staticmethod
    cdef void c_callback(void *arg, adcusb_device_t dev, adcusb_data_block *blk) with gil:
        cdef object self = <object>arg
        cdef adcusb_data_block *copied
        cdef ADCDataBlock block

        if blk == NULL:
            if callable(self.callback):
                self.callback(None)

            return

        block_size = sizeof(adcusb_data_block) + blk.adb_count * sizeof(uint32_t)

        if callable(self.callback):
            with nogil:
                copied = <adcusb_data_block *>malloc(block_size)
                memcpy(copied, blk, block_size)

            block = ADCDataBlock.__new__(ADCDataBlock)
            block.block = copied
            block.init()
            self.callback(block)
