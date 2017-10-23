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

from libc.stdint cimport *

ctypedef void (*adcusb_callback_t)(void *arg, adcusb_device_t, adcusb_data_block *);

cdef extern from "adcusb.h" nogil:
    ctypedef struct adcusb_device_t:
        pass

    cdef struct adcusb_data_block:
        uint64_t adb_seqno
        uint32_t adb_overrun
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
