#ifndef LIBADCUSB_H
#define LIBADCUSB_H

#include <libusb-1.0/libusb.h>
#include <glib.h>

struct adcusb_device;
typedef struct adcusb_device adcusb_device_t;

struct adcusb_data_block
{

};

typedef void (^adcusb_callback_t)(struct adcusb_device *,
    struct adcusb_data_block *, void *);

int adcusb_open_by_serial(const char *serial, struct adcusb_device **device);
int adcusb_open_by_address(int address);
void adcusb_set_callback(struct adcusb_device *dev, adcusb_callback_t cb,
    void *arg);
int acdusb_start(struct adcusb_device *dev);
void adcusb_stop(struct adcusb_device *dev);
void adcusb_close(struct adcusb_device *dev);

#endif