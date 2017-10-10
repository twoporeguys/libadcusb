#
# Copyright 2017 Two Pore Guys, Inc.
# All rights reserved.
#
# This is proprietary software.
#

from libc.stdint cimport *

ctypedef void (*adcusb_callback_t)(void *arg, adcusb_device_t, adcusb_data_block *);

cdef extern from "adcusb.h" nogil:
    ctypedef struct adcusb_device_t:
        pass

    cdef struct adcusb_data_block:
        uint64_t adb_seqno
        uint32_t adb_count
        uint32_t adb_samples[0]

    void *ADCUSB_CALLBACK(adcusb_callback_t cb, void *arg);

    int adcusb_open_by_serial(const char *serial, adcusb_device_t *devp)
    int adcusb_open_by_address(int address, adcusb_device_t *devp)
    void adcusb_set_buffer_size(adcusb_device_t dev, size_t bufsize)
    void adcusb_set_callback(adcusb_device_t dev, void *cb)
    int adcusb_start(adcusb_device_t dev)
    void adcusb_stop(adcusb_device_t dev)
    void adcusb_close(adcusb_device_t dev)
