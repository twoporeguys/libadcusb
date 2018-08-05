#!/bin/bash

case "`uname -s`" in
    Darwin*)
	if [ -x /usr/local/bin/brew ]; then
        	echo "I am a Mac.  Brew installing stuff."
		brew install cmake glib gtk+3 libusb python3 pkg-config
		brew install pygobject3 --with-python3
	elif [ -x /opt/local/bin/port ]; then
        	echo "I am a Mac.  Using macports to install stuff with sudo"
        	sudo port install python36 py36-pip cmake glib2 gtk3 py36-gobject pkgconfig
		sudo port select --set pip pip36
		sudo ln -fs /opt/local/bin/pip-3.6 /opt/local/bin/pip3
	else
		echo "I am a Mac but you have neither macports or brew installed."
		exit 1
	fi
	sudo pip3 install -r requirements.txt
        ;;
    Linux*)
        echo "I am some kind of Linux, hopefully Ubuntu."
        apt-get -y install \
	    cmake clang libglib2.0-dev libusb-1.0-0-dev python3 python3-pip \
	    python3-numpy python3-sphinx cython3 doxygen
        ;;
    default)
        echo "Don't know how to set up for your OS."
        exit 1
esac

