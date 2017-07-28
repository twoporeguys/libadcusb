#include "adcusb.h"

#include <libusb-1.0/libusb.h>

struct adcusb_device
{
	libusb_context *	ad_libusb;
	libusb_device_handle *	ad_handle;
	GThread *		ad_libusb_thread;
};

int
adcusb_open_by_serial(const char *serial, struct adcusb_device **device)
{

}

int
adcusb_open_by_address(int address)
{

}

void
adcusb_set_callback(struct adcusb_device *dev, adcusb_callback_t cb, void *arg)
{

}

int
acdusb_start(struct adcusb_device *dev)
{

}

void
adcusb_stop(struct adcusb_device *dev)
{

}

void
adcusb_close(struct adcusb_device *dev)
{

}
