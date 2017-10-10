#
# Copyright 2017 Two Pore Guys, Inc.
# All rights reserved.
#
# This is proprietary software.
#

import sys
import signal
import adcusb


def cb(block):
    print('Block: seqno={0}, length={1}'.format(block.seqno, block.length))
    print(block.samples.__array__())


def main():
    if len(sys.argv) < 2:
        print('Usage: pyread <Device serial number>')
        sys.exit(1)

    adc = adcusb.ADC(serial=sys.argv[1])
    adc.callback = cb
    adc.start()
    signal.pause()


if __name__ == '__main__':
    main()
