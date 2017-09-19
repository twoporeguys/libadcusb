#!/bin/bash

case $OSTYPE in
    darwin*)
        echo "I am a Mac.  Brew installing stuff."
        echo "Don't worry about warnings of things already installed."
        brew install --upgrade cmake glib libusb python3
        pip3 install cython numpy scipy
        ;;
    default)
        echo "Don't know how to set up for your OS."
        exit 1
esac
