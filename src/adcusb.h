#ifndef LIBADCUSB_H
#define LIBADCUSB_H

#include <libusb-1.0/libusb.h>
#include <glib.h>

struct adcusb_device;
typedef struct adcusb_device *adcusb_device_t;

struct adcusb_data_block
{
	uint64_t	adb_seqno;
	uint32_t 	adb_count;
	uint32_t 	adb_samples[];

};

typedef void (^adcusb_callback_t)(struct adcusb_device *,
    struct adcusb_data_block *);

#define	ADCUSB_CALLBACK(_fn, _arg) 					\
    ^(struct adcusb_device *_dev, struct adcusb_data_block *_blk) {	\
        _fn(_arg, _dev, _blk);						\
    }

int adcusb_open_by_serial(const char *serial, adcusb_device_t *devp);
int adcusb_open_by_address(int address, adcusb_device_t *devp);
void adcusb_set_buffer_size(adcusb_device_t dev, size_t bufsize);
void adcusb_set_callback(adcusb_device_t dev, adcusb_callback_t cb);
int adcusb_start(adcusb_device_t dev);
void adcusb_stop(adcusb_device_t dev);
void adcusb_close(adcusb_device_t dev);

#endif
