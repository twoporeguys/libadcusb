/*
 * Copyright 2017 Two Pore Guys, Inc.
 * All rights reserved.
 *
 * This is proprietary software.
 */

#ifndef LIBADCUSB_H
#define LIBADCUSB_H

#include <libusb-1.0/libusb.h>

#define ADBUSB_SERIAL_MAX_LEN   (255)

struct adcusb_device;
typedef struct adcusb_device *adcusb_device_t;

struct adcusb_data_block
{
	uint64_t	adb_seqno;
	uint32_t	adb_overrun;
	uint32_t 	adb_count;
	uint32_t 	adb_samples[];

};

typedef void (^adcusb_callback_t)(struct adcusb_device *,
    struct adcusb_data_block *);

#define	ADCUSB_CALLBACK(_fn, _arg) 					\
	^(struct adcusb_device *_dev, struct adcusb_data_block *_blk) {	\
		_fn(_arg, _dev, _blk);					\
	}

/** Queries the available USB devices and returns an array of serial numbers for each device found.
    Pass one of the returned serial number strings to adcusb_open_by_serial.
    @param serials (input) Array of client allocated char buffers to receive
     the list of available usb device serial numbers.
     Each buffer should be 'ADBUSB_SERIAL_MAX_LEN' in size.
    @param serial_count (input) number of elements in the serials buffer array.
    @returns the number of devices found and placed in the serials array
    - or - zero if no devices found.
*/
int adcusb_get_device_serial_numbers(char serials[][20], int serial_count);
int adcusb_open_by_serial(const char *serial, adcusb_device_t *devp);
int adcusb_open_by_address(int address, adcusb_device_t *devp);
void adcusb_set_buffer_size(adcusb_device_t dev, size_t bufsize);
void adcusb_set_callback(adcusb_device_t dev, adcusb_callback_t cb);
int adcusb_start(adcusb_device_t dev);
void adcusb_stop(adcusb_device_t dev);
void adcusb_close(adcusb_device_t dev);

#endif	/* LIBADCUSB_H */
