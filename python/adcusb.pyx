#
# Copyright 2017 Two Pore Guys, Inc.
# All rights reserved.
#
# This is proprietary software.
#

import os
import numpy as np
cimport numpy as np
from libc.stdint cimport *
from libc.errno cimport errno
from cpython cimport PyObject, Py_INCREF
from adcusb cimport *


np.import_array()


class ADCException(RuntimeError):
    pass


cdef class ADCArray(object):
    cdef uint32_t *samples
    cdef uint32_t count

    def __array__(self):
        cdef np.npy_intp shape[1]

        shape[0] = <np.npy_intp>self.count
        ndarray = np.PyArray_SimpleNewFromData(1, shape, np.NPY_FLOAT64, self.samples)
        return ndarray

    def __dealloc__(self):
        pass


cdef class ADCDataBlock(object):
    cdef adcusb_data_block *block

    def __dealloc__(self):
        pass #free(<void *>self.block)

    property seqno:
        def __get__(self):
            return self.block.adb_seqno

    property length:
        def __get__(self):
            return self.block.adb_count

    property samples:
        def __get__(self):
            cdef ADCArray arr

            arr = ADCArray.__new___(ADCArray)
            arr.samples = self.block.adb_samples
            arr.count = self.block.adb_count
            return arr


cdef class ADC(object):
    cdef adcusb_device_t dev
    cdef public object callback

    def __init__(self, address=None, serial=None):
        cdef const char *c_serial
        cdef int c_address
        cdef int ret

        if not address and not serial:
            raise ADCException('Address or serial must be provided')

        if address:
            u_serial = serial.encode('utf-8')
            c_serial = u_serial
            with nogil:
                ret = adcusb_open_by_serial(c_serial, &self.dev)

        elif serial:
            c_address = address
            with nogil:
                ret = adcusb_open_by_address(c_address, &self.dev)

        if ret != 0:
            raise ADCException(os.strerror(errno))

    def __dealloc__(self):
        with nogil:
            adcusb_close(self.dev)

    def start(self):
        with nogil:
            adcusb_start(self.dev)

    def stop(self):
        with nogil:
            adcusb_stop(self.dev)

    @staticmethod
    cdef void c_callback(void *arg, adcusb_device_t dev, adcusb_data_block *blk) with gil:
        cdef object self = <object>arg
        cdef ADCDataBlock block

        if callable(self.callback):
            block = ADCDataBlock.__new__(ADCDataBlock)
            block.block = blk
            self.callback(block)
