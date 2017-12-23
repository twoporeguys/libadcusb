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
	int32_t 	adb_samples[];	/**< Variable array of samples */

};

/** ADC block callback type */
typedef void (^adcusb_callback_t)(struct adcusb_device *_Nonnull,
    struct adcusb_data_block *_Nullable);

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
int adcusb_open_by_serial(const char *_Nonnull serial,
    _Nonnull adcusb_device_t *_Nonnull devp);

/**
 * Opens a device by its address.
 *
 * @param address
 * @param devp
 * @return
 */
int adcusb_open_by_address(int address,
    _Nonnull adcusb_device_t *_Nonnull devp);

/**
 * Sets the buffer size.
 *
 * Buffer size is always expressed in blocks, not bytes.
 *
 * @param dev
 * @param bufsize Buffer size in blocks
 */
void adcusb_set_buffer_size(_Nonnull adcusb_device_t dev, size_t bufsize);

/**
 * Sets the block callback function.
 *
 * Existing callback can be unset by passing NULL as "cb".
 *
 * @param dev Device handle
 * @param cb Callback function or NULL
 */
void adcusb_set_callback(_Nonnull adcusb_device_t dev,
    _Nullable adcusb_callback_t cb);

/**
 * Starts data acquisition.
 *
 * @param dev Device handle
 * @return 0 on success, -1 on error.
 */
int adcusb_start(_Nonnull adcusb_device_t dev);

/**
 * Stops data acquisition.
 *
 * @param dev Device handle
 */
void adcusb_stop(_Nonnull adcusb_device_t dev);

/**
 * Closes device handle
 *
 * @param dev Device handle
 */
void adcusb_close(_Nonnull adcusb_device_t dev);

#endif	/* LIBADCUSB_H */
