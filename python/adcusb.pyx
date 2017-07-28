from adcusb cimport *

cdef class ADC(object):
    cdef adcusb_device_t dev

    def ADC(self, address=None, serial=None):
        pass

    def __dealloc__(self):
        pass

    def start(self):
        with nogil:
            adcusb_start(self.dev)

    def stop(self):
        with nogil:
            adcusb_stop(self.dev)

    cdef void c_callback(void *arg, adcusb_device_t dev, adcusb_data_block *blk):
        pass
