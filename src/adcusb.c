#include <Block.h>
#include <stdint.h>
#include <stdbool.h>
#include <glib.h>
#include <libusb-1.0/libusb.h>
#include "adcusb.h"

static void adcusb_transfer_cb(struct libusb_transfer *);
static void *adcusb_libusb_thread(void *);

struct adcusb_packet
{

};

struct adcusb_device
{
	libusb_context *	ad_libusb;
	libusb_device_handle *	ad_handle;
	struct libusb_transfer *ad_xfer;
	GThread *		ad_libusb_thread;
	bool 			ad_running;
	bool			ad_transfer;
	adcusb_callback_t 	ad_callback;
	struct adcusb_packet *	ad_buffer;
	int 			ad_buffer_size;
};

int
adcusb_open_by_serial(const char *serial, struct adcusb_device **devp)
{
	struct adcusb_device *dev = g_malloc0(sizeof(*dev));
	struct libusb_device **devices;
	struct libusb_device_descriptor desc;
	uint8_t strdesc[NAME_MAX];

	if (libusb_init(&dev->ad_libusb) != 0) {
		g_free(dev);
		return (-1);
	}

	libusb_get_device_list(dev->ad_libusb, &devices);

	for (; *devices != NULL; devices++) {
		if (libusb_get_device_descriptor(*devices, &desc) != 0)
			continue;

		if (libusb_open(*devices, &dev->ad_handle) != 0)
			goto fail;

		if (libusb_get_string_descriptor_ascii(dev->ad_handle,
		    desc.iSerialNumber, strdesc, sizeof(strdesc)) < 0) {
			libusb_close(dev->ad_handle);
			continue;
		}

		if (g_strcmp0((char *)strdesc, serial) != 0)
			continue;

		if (libusb_claim_interface(dev->ad_handle, 1) != 0) {
			libusb_close(dev->ad_handle);
			goto fail;
		}

		*devp = dev;
		return (0);
	}

fail:
	libusb_exit(dev->ad_libusb);
	g_free(dev);
	return (-1);
}

int
adcusb_open_by_address(int address, struct adcusb_device **devp)
{
	struct adcusb_device *dev = g_malloc0(sizeof(*dev));
	struct libusb_device **devices;

	if (libusb_init(&dev->ad_libusb) != 0) {
		g_free(dev);
		return (-1);
	}

	libusb_get_device_list(dev->ad_libusb, &devices);

	for (; *devices != NULL; devices++) {
		if (libusb_get_device_address(*devices) != address)
			continue;

		if (libusb_open(*devices, &dev->ad_handle) != 0)
			goto fail;

		if (libusb_claim_interface(dev->ad_handle, 1) != 0) {
			libusb_close(dev->ad_handle);
			goto fail;
		}

		*devp = dev;
		return (0);
	}

fail:
	libusb_exit(dev->ad_libusb);
	g_free(dev);
	return (-1);
}

void
adcusb_set_callback(struct adcusb_device *dev, adcusb_callback_t cb)
{

	dev->ad_callback = Block_copy(cb);
}

int
adcusb_start(struct adcusb_device *dev)
{

	dev->ad_libusb_thread = g_thread_new("adcusb", adcusb_libusb_thread, dev);
	dev->ad_transfer = true;
	dev->ad_buffer = g_malloc0(10 * 576);
	dev->ad_xfer = libusb_alloc_transfer(1);
	dev->ad_buffer_size = 5760;

	libusb_fill_iso_transfer(dev->ad_xfer, dev->ad_handle,
	    2 | LIBUSB_ENDPOINT_IN,
	    (uint8_t *)dev->ad_buffer, 576, 1,
	    adcusb_transfer_cb, dev, 1000);

	libusb_set_iso_packet_lengths(dev->ad_xfer, 576);

	if (libusb_submit_transfer(dev->ad_xfer) != 0)
		return (-1);

	return (0);
}

void
adcusb_stop(struct adcusb_device *dev)
{
	dev->ad_transfer = false;
}

void
adcusb_close(struct adcusb_device *dev)
{

}

static void
adcusb_transfer_cb(struct libusb_transfer *xfer)
{
	struct adcusb_device *dev = xfer->user_data;
	struct libusb_iso_packet_descriptor *iso;
	int i;

	for (i = 0; i < xfer->num_iso_packets; i++) {
		iso = &xfer->iso_packet_desc[i];
		dev->ad_callback(dev, NULL);
	}

	libusb_submit_transfer(xfer);
}

static void *
adcusb_libusb_thread(void *arg)
{
	struct adcusb_device *dev = arg;

	while (dev->ad_running)
		libusb_handle_events(dev->ad_libusb);

	return (NULL);
}
