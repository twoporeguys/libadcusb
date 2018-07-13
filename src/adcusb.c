/*+
 * Copyright 2017 Two Pore Guys, Inc.
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <Block.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <glib.h>
#include <libusb.h>
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
static inline int adcusb_open(adcusb_device_t *, const char *, int);

struct adcusb_device
{
	libusb_context *	ad_libusb;
	libusb_device_handle *	ad_handle;
	struct libusb_transfer *ad_xfers[ADCUSB_NUM_XFERS];
	GThread *		ad_libusb_thread;
	bool 			ad_open;
	bool			ad_running;
	bool			ad_dead;
	adcusb_callback_t 	ad_callback;
	struct adcusb_packet *	ad_buffers[ADCUSB_NUM_XFERS];
	int 			ad_buffer_size;
    	int 			ad_active_xfers_cnt;
	size_t			ad_num_descs;
	GMutex			ad_mtx;
	GCond			ad_cv;
};

static inline int adcusb_open(adcusb_device_t *devp, const char *serial, int address) {
	struct adcusb_device *dev = g_malloc0(sizeof(*dev));
	struct libusb_device **devices;
	struct libusb_device_descriptor desc;
	uint8_t strdesc[NAME_MAX];

	dev->ad_num_descs = ADCUSB_NUM_DESCS;
	dev->ad_open = true;

	if (libusb_init(&dev->ad_libusb) != 0) {
		g_free(dev);
		return (-1);
	}

	libusb_get_device_list(dev->ad_libusb, &devices);

	g_mutex_init(&dev->ad_mtx);
	g_cond_init(&dev->ad_cv);

	for (; *devices != NULL; devices++) {

		if (serial == NULL) {
			if (libusb_get_device_address(*devices) != address)
				continue;

			if (libusb_open(*devices, &dev->ad_handle) != 0)
				goto fail;
		} else {
			if (libusb_get_device_descriptor(*devices, &desc) != 0)
				continue;

			if (libusb_open(*devices, &dev->ad_handle) != 0)
				goto fail;

			if (libusb_get_string_descriptor_ascii(dev->ad_handle,
			    desc.iSerialNumber, strdesc, sizeof(strdesc)) < 0) {
				libusb_close(dev->ad_handle);
				continue;
			}

			if (g_strcmp0((char *) strdesc, serial) != 0) {
				libusb_close(dev->ad_handle);
				continue;
			}
		}

		*devp = dev;
		dev->ad_libusb_thread = g_thread_new("adcusb",
		    adcusb_libusb_thread, dev);
		return (0);
	}

fail:
	libusb_exit(dev->ad_libusb);
	g_cond_clear(&dev->ad_cv);
	g_mutex_clear(&dev->ad_mtx);
	g_free(dev);
	return (-1);
}

int
adcusb_open_by_serial(const char *serial, adcusb_device_t *devp)
{

	return adcusb_open(devp, serial, 0);
}

int
adcusb_open_by_address(int address, adcusb_device_t *devp)
{

	return adcusb_open(devp, NULL, address);
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
	g_mutex_lock(&dev->ad_mtx);

	g_assert_nonnull(dev);

	if (!dev->ad_open) {
		errno = ENXIO;
		goto fail;
	}

	if (dev->ad_dead) {
		errno = EINVAL;
		goto fail;
	}

	if (dev->ad_running)
		goto done;

	if (libusb_claim_interface(dev->ad_handle, 1) != 0) {
		errno = EPERM;
		goto fail;
	}

	dev->ad_running = true;

	for (int i = 0; i < ADCUSB_NUM_XFERS; i++) {
		dev->ad_buffers[i] = g_malloc0(dev->ad_num_descs * ADCUSB_PACKET_SIZE);
		dev->ad_xfers[i] = libusb_alloc_transfer(ADCUSB_PACKET_SIZE);
		dev->ad_buffer_size = (int)(ADCUSB_PACKET_SIZE * dev->ad_num_descs);

		libusb_fill_iso_transfer(dev->ad_xfers[i], dev->ad_handle,
		    ADCUSB_EP_NUM | LIBUSB_ENDPOINT_IN,
		    (uint8_t *)dev->ad_buffers[i], ADCUSB_PACKET_SIZE,
		    (int)dev->ad_num_descs, adcusb_transfer_cb, dev, 1000);

		libusb_set_iso_packet_lengths(dev->ad_xfers[i], ADCUSB_PACKET_SIZE);

		if (libusb_submit_transfer(dev->ad_xfers[i]) != 0) {
			errno = EXDEV;
			goto fail;
		}

		dev->ad_active_xfers_cnt++;
	}

done:
	g_mutex_unlock(&dev->ad_mtx);
	return (0);
fail:
	dev->ad_running = false;
	g_mutex_unlock(&dev->ad_mtx);
	return (-1);
}

void
adcusb_stop(struct adcusb_device *dev)
{
	g_mutex_lock(&dev->ad_mtx);

	if (!dev->ad_running)
		goto done;

	dev->ad_running = false;

	while (dev->ad_active_xfers_cnt > 0)
		g_cond_wait(&dev->ad_cv, &dev->ad_mtx);

done:
	g_mutex_unlock(&dev->ad_mtx);
}

void
adcusb_close(struct adcusb_device *dev)
{
	g_assert_nonnull(dev);
	g_assert_nonnull(dev->ad_handle);
	g_assert_nonnull(dev->ad_libusb);

	dev->ad_open = false;

	g_mutex_lock(&dev->ad_mtx);

	libusb_release_interface(dev->ad_handle, 1);
	libusb_close(dev->ad_handle);
	g_thread_join(dev->ad_libusb_thread);
	libusb_exit(dev->ad_libusb);

	if (dev->ad_callback != NULL)
		Block_release(dev->ad_callback);

	g_mutex_unlock(&dev->ad_mtx);
	g_cond_clear(&dev->ad_cv);
	g_mutex_clear(&dev->ad_mtx);
	g_free(dev);
}

static void
adcusb_transfer_cb(struct libusb_transfer *xfer)
{
	struct adcusb_device *dev = xfer->user_data;
	struct adcusb_data_block *block;
	struct libusb_iso_packet_descriptor *iso;
	int i;
	g_mutex_lock(&dev->ad_mtx);

	if (xfer->status != LIBUSB_TRANSFER_COMPLETED)
		goto unplug;

	for (i = 0; i < xfer->num_iso_packets; i++) {
		iso = &xfer->iso_packet_desc[i];
		if (iso->actual_length < sizeof(struct adcusb_data_block))
			continue;

		block = (struct adcusb_data_block *)
		    &xfer->buffer[ADCUSB_PACKET_SIZE * i];

		if (block->adb_count == 0)
			continue;

		dev->ad_callback(dev, block);
	}

	if (dev->ad_running) {
		if (libusb_submit_transfer(xfer) != 0)
			goto unplug;
	} else {
		dev->ad_active_xfers_cnt--;
		g_cond_signal(&dev->ad_cv);
	}

	g_mutex_unlock(&dev->ad_mtx);
	return;

unplug:
	if (xfer->status == LIBUSB_TRANSFER_NO_DEVICE)
		dev->ad_dead = true;

	dev->ad_active_xfers_cnt--;
	dev->ad_callback(dev, NULL);
	g_free(xfer->buffer);
	libusb_free_transfer(xfer);
	g_cond_signal(&dev->ad_cv);
	g_mutex_unlock(&dev->ad_mtx);
}

static void *
adcusb_libusb_thread(void *arg)
{
	struct adcusb_device *dev = arg;

	while (dev->ad_open)
		libusb_handle_events(dev->ad_libusb);

	return (NULL);
}
