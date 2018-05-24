#!/bin/bash

case "`uname -s`" in
    Darwin*)
	if [ -x /usr/local/bin/brew ]; then
        	echo "I am a Mac.  Brew installing stuff."
		brew install cmake glib gtk+3 libusb python3
		brew install pygobject3 --with-python3
		pip3 install -r requirements.txt
	elif [ -x /opt/local/bin/port ]; then
        	echo "I am a Mac.  Using macports to install stuff with sudo"
        	sudo port install python36 py36-pip cmake glib2 gtk3 py36-gobject
		sudo pip-3.6 install -r requirements.txt
	else
		echo "I am a Mac but you have neither macports or brew installed."
		exit 1
	fi
        ;;
    Linux*)
        echo "I am some kind of Linux, hopefully Ubuntu."
        apt-get -y install python3 cmake gtk+3
        pip3 install -r requirements.txt
        ;;
    default)
        echo "Don't know how to set up for your OS."
        exit 1
esac
