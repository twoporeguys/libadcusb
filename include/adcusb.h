/*
 * Copyright 2017 Two Pore Guys, Inc.
 * All rights reserved.
 *
 * This is proprietary software.
 */

#ifndef LIBADCUSB_H
#define LIBADCUSB_H

/**
 * @file adcusb.h
 *
 * ADC USB library interface
 */

struct adcusb_device;

/** ADC device handle */
typedef struct adcusb_device *adcusb_device_t;

/**
 * ADC data block
 */
struct adcusb_data_block
{
	uint64_t	adb_seqno;	/**< Sequence number */
	uint32_t	adb_overrun;	/**< Overrun counter */
	uint32_t 	adb_count;	/**< Number of samples in the block */
	uint32_t 	adb_samples[];	/**< Variable array of samples */

};

/** ADC block callback type */
typedef void (^adcusb_callback_t)(struct adcusb_device *,
    struct adcusb_data_block *);

/** Converts ordinary function pointer into a Block */
#define	ADCUSB_CALLBACK(_fn, _arg) 					\
	^(struct adcusb_device *_dev, struct adcusb_data_block *_blk) {	\
		_fn(_arg, _dev, _blk);					\
	}

/**
 * Opens a device by its serial number.
 *
 * @param serial
 * @param devp
 * @return
 */
int adcusb_open_by_serial(const char *serial, adcusb_device_t *devp);

/**
 * Opens a device by its address.
 *
 * @param address
 * @param devp
 * @return
 */
int adcusb_open_by_address(int address, adcusb_device_t *devp);

/**
 * Sets the buffer size.
 *
 * Buffer size is always expressed in blocks, not bytes.
 *
 * @param dev
 * @param bufsize Buffer size in blocks
 */
void adcusb_set_buffer_size(adcusb_device_t dev, size_t bufsize);

/**
 * Sets the block callback function.
 *
 * Existing callback can be unset by passing NULL as "cb".
 *
 * @param dev Device handle
 * @param cb Callback function or NULL
 */
void adcusb_set_callback(adcusb_device_t dev, adcusb_callback_t cb);

/**
 * Starts data acquisition.
 *
 * @param dev Device handle
 * @return 0 on success, -1 on error.
 */
int adcusb_start(adcusb_device_t dev);

/**
 * Stops data acquisition.
 *
 * @param dev Device handle
 */
void adcusb_stop(adcusb_device_t dev);

/**
 * Closes device handle
 *
 * @param dev Device handle
 */
void adcusb_close(adcusb_device_t dev);

#endif	/* LIBADCUSB_H */
