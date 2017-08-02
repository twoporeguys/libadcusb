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
#include "../../src/adcusb.h"

static void
usage(const char *argv0)
{

	fprintf(stderr, "Usage: %s [-s serial|-d address] <output>\n", argv0);
	fprintf(stderr, "\n");
	fprintf(stderr, "-s serial -- opens adcusb device by serial number\n");
	fprintf(stderr, "-d address -- opens adcusb device by USB address\n");
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

	while ((opt = getopt(argc, argv, "hs:d:")) != -1) {
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
		}
	}

	if (address == NULL && serial == NULL) {
		fprintf(stderr, "Please specify -s or -d\n");
		return (1);
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

	adcusb_set_callback(dev, ADCUSB_CALLBACK(callback, NULL));

	if (adcusb_start(dev) != 0) {
		fprintf(stderr, "Failed to start data acquisition: %s [%d]\n",
		    strerror(errno), errno);
		return (1);
	}

	printf("Started data acquisition\n");
	pause();
}
