/*
 * Copyright 2017 Two Pore Guys, Inc.
 * All rights reserved.
 *
 * This is proprietary software.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <adcusb.h>
#include <rpc/client.h>

static void
usage(const char *argv0)
{

	fprintf(stderr, "Usage: %s [-l | -s serial | -d address] <output>\n", argv0);
	fprintf(stderr, "\n");
	fprintf(stderr, "-s serial -- opens adcusb device by serial number\n");
	fprintf(stderr, "-d address -- opens adcusb device by USB address\n");
	fprintf(stderr, "-l address -- list adcusb device by serial number\n");
}

static void
callback(void *arg, adcusb_device_t dev, struct adcusb_data_block *block)
{

	(void)arg;
	(void)dev;
	printf("Read block: %u samples, seqno %" PRIu64 "\n", block->adb_count,
	    block->adb_seqno);
}

int
main(int argc, char *argv[])
{
	int opt;
	char *serial = NULL;
	char *address = NULL;
	uint32_t addr_int;
	adcusb_device_t dev;
    int list = 0;
    int fds[2]; /* fds[0] is read end, fds[1] is write end */

	while ((opt = getopt(argc, argv, "hls:d:")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
			return (1);

		case 's':
			serial = strdup(optarg);
			break;

		case 'd':
			address = strdup(optarg);
			break;

		case 'l':
			list = 1;
			break;
		}
	}

	if (list == 0 && address == NULL && serial == NULL) {
		fprintf(stderr, "Please specify -h -l -s or -d\n");
		return (1);
	}

    if (list == 1) {
        #define SERIAL_COUNT  (20)
        char serials[ADBUSB_SERIAL_MAX_LEN][SERIAL_COUNT];
        int i, count = adcusb_get_device_serial_numbers(serials, SERIAL_COUNT);
        printf("%d devices found:\n", count);
        for (i = 0; i < count; i++)
        {
            printf("%s\n", serials[i]);
        }
        return (0);
    }

	if (serial != NULL) {
		if (adcusb_open_by_serial(serial, &dev) != 0) {
			fprintf(stderr, "Cannot open device: %s [%d]\n",
			    strerror(errno), errno);
			return (1);
		}

		printf("Opened device <serial:%s>\n", serial);
	}

	if (address != NULL) {
		addr_int = (uint32_t)strtoul(address, NULL, 10);
		if (adcusb_open_by_serial(serial, &dev) != 0) {
			fprintf(stderr, "Cannot open device: %s [%d]\n",
			    strerror(errno), errno);
			return (1);
		}

		printf("Opened device <address:%u>\n", addr_int);
	}

    pipe(fds);
    rpc_client_t client = rpc_client_create("usb://MOMtest", rpc_fd_create(fds[1]));
    rpc_connection_t conn = rpc_client_get_connection(client);
    if (conn) {
        printf("Calling fread \"start\"\n");
        rpc_connection_call_sync(conn, "fread", "start", NULL);
    }

	adcusb_set_callback(dev, ADCUSB_CALLBACK(callback, NULL));

	if (adcusb_start(dev) != 0) {
		fprintf(stderr, "Failed to start data acquisition: %s [%d]\n",
		    strerror(errno), errno);
		return (1);
	}

	printf("Started data acquisition\n");

#ifdef _WIN32
	for (;;)
		sleep(1);
#else
	pause();
#endif
    return (0);
}
