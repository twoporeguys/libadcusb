export CC := clang
export CXX := clang++
BUILD_PYTHON := ON
PYTHON_VERSION ?= "python3"
PREFIX ?= "/usr/local"
BUILD_TYPE ?= Release
ENABLE_RPATH ?= ON

.PHONY: all build clean bootstrap install uninstall

all: build

build:
	mkdir -p build
	cd build && cmake .. \
	    -DPYTHON_VERSION=$(PYTHON_VERSION) \
	    -DCMAKE_INSTALL_PREFIX=$(PREFIX) \
	    -DBUILD_PYTHON=$(BUILD_PYTHON) \
	    -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	    -DENABLE_RPATH=$(ENABLE_RPATH)
	make

clean:
	rm -rf *~ build

bootstrap:
	sh requirements.sh

install:
	make -C build install

uninstall:
	make -C build uninstall
