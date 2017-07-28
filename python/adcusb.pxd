cdef extern from "adcusb.h":
    ctypedef struct adcusb_device_t:
        pass

    ctypedef struct adcusb_data_block:
        uint32_t adb_seqno
        int adb_format

    void *ADCUSB_CALLBACK(void *arg)

    int adcusb_open_by_serial(const char *serial, adcusb_device_t *devp)
    int adcusb_open_by_address(int address, adcusb_device_t *devp)
    void adcusb_set_buffer_size(adcusb_device_t dev, size_t bufsize)
    void adcusb_set_callback(adcusb_device_t dev, adcusb_callback_t cb)
    int acdusb_start(adcusb_device_t dev)
    void adcusb_stop(adcusb_device_t dev)
    void adcusb_close(adcusb_device_t dev)
