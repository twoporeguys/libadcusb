/*
 * Copyright 2017 Two Pore Guys, Inc.
 * All rights reserved.
 *
 * This is proprietary software.
 */

#include <Block.h>
#include <stdint.h>
#include <stdbool.h>
#include <glib.h>
#include <libusb-1.0/libusb.h>
#include <adcusb.h>

#ifndef NAME_MAX
#define NAME_MAX	255
#endif

#define	ADCUSB_PACKET_SIZE	576
#define	ADCUSB_NUM_XFERS	4
#define	ADCUSB_NUM_DESCS	64
#define	ADCUSB_EP_NUM		1

static void adcusb_transfer_cb(struct libusb_transfer *);
static void *adcusb_libusb_thread(void *);

struct adcusb_device
{
	libusb_context *	ad_libusb;
	libusb_device_handle *	ad_handle;
	struct libusb_transfer *ad_xfers[ADCUSB_NUM_XFERS];
	GThread *		ad_libusb_thread;
	bool 			ad_running;
	bool			ad_transfer;
	adcusb_callback_t 	ad_callback;
	struct adcusb_packet *	ad_buffers[ADCUSB_NUM_XFERS];
	int 			ad_buffer_size;
	size_t			ad_num_descs;
};

int
adcusb_open_by_serial(const char *serial, adcusb_device_t *devp)
{
	struct adcusb_device *dev = g_malloc0(sizeof(*dev));
	struct libusb_device **devices;
	struct libusb_device_descriptor desc;
	uint8_t strdesc[NAME_MAX];

	dev->ad_num_descs = ADCUSB_NUM_DESCS;

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

		if (g_strcmp0((char *)strdesc, serial) != 0) {
			libusb_close(dev->ad_handle);
			continue;
		}

		if (libusb_claim_interface(dev->ad_handle, 0) != 0) {
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
adcusb_open_by_address(int address, adcusb_device_t *devp)
{
	struct adcusb_device *dev = g_malloc0(sizeof(*dev));
	struct libusb_device **devices;

	if (libusb_init(&dev->ad_libusb) != 0) {
		g_free(dev);
		return (-1);
	}

	dev->ad_num_descs = ADCUSB_NUM_DESCS;

	libusb_get_device_list(dev->ad_libusb, &devices);

	for (; *devices != NULL; devices++) {
		if (libusb_get_device_address(*devices) != address)
			continue;

		if (libusb_open(*devices, &dev->ad_handle) != 0)
			goto fail;

		if (libusb_claim_interface(dev->ad_handle, 0) != 0) {
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
	g_assert_nonnull(dev);

	if (dev->ad_callback != NULL)
		Block_release(dev->ad_callback);

	dev->ad_callback = cb != NULL ? Block_copy(cb) : NULL;
}

int
adcusb_start(struct adcusb_device *dev)
{
	g_assert_nonnull(dev);

	dev->ad_running = true;
	dev->ad_libusb_thread = g_thread_new("adcusb", adcusb_libusb_thread, dev);
	dev->ad_transfer = true;

	for (int i = 0; i < ADCUSB_NUM_XFERS; i++) {
		dev->ad_buffers[i] = g_malloc0(dev->ad_num_descs * ADCUSB_PACKET_SIZE);
		dev->ad_xfers[i] = libusb_alloc_transfer(ADCUSB_PACKET_SIZE);
		dev->ad_buffer_size = (int)(ADCUSB_PACKET_SIZE * dev->ad_num_descs);

		libusb_fill_iso_transfer(dev->ad_xfers[i], dev->ad_handle,
		    ADCUSB_EP_NUM | LIBUSB_ENDPOINT_IN,
		    (uint8_t *)dev->ad_buffers[i], ADCUSB_PACKET_SIZE,
		    (int)dev->ad_num_descs, adcusb_transfer_cb, dev, 1000);

		libusb_set_iso_packet_lengths(dev->ad_xfers[i], ADCUSB_PACKET_SIZE);

		if (libusb_submit_transfer(dev->ad_xfers[i]) != 0)
			return (-1);
	}

	return (0);
}

void
adcusb_stop(struct adcusb_device *dev)
{
	dev->ad_transfer = false;

	for (int i = 0; i < ADCUSB_NUM_XFERS; i++) {
		libusb_free_transfer(dev->ad_xfers[i]);
		g_free(dev->ad_buffers[i]);
	}
}

void
adcusb_close(struct adcusb_device *dev)
{
	g_assert_nonnull(dev);

	if (dev->ad_callback != NULL)
		Block_release(dev->ad_callback);

	libusb_close(dev->ad_handle);
	libusb_exit(dev->ad_libusb);
	g_free(dev);
}

static void
adcusb_transfer_cb(struct libusb_transfer *xfer)
{
	struct adcusb_device *dev = xfer->user_data;
	struct adcusb_data_block *block;
	struct libusb_iso_packet_descriptor *iso;
	int i;

	for (i = 0; i < xfer->num_iso_packets; i++) {
		iso = &xfer->iso_packet_desc[i];
		block = (struct adcusb_data_block *)
		    &xfer->buffer[ADCUSB_PACKET_SIZE * i];
		dev->ad_callback(dev, block);
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
