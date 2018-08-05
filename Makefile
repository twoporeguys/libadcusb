export CC := clang
export CXX := clang++
BUILD_PYTHON := ON
PYTHON_VERSION ?= "python3"
PREFIX ?= "/usr/local"

.PHONY: all build clean bootstrap install uninstall

all: build

build:
	mkdir -p build 
	cd build 
	cd build && cmake .. \
	    -DPYTHON_VERSION=${PYTHON_VERSION} \
	    -DCMAKE_INSTALL_PREFIX=${PREFIX} \
	    -DBUILD_PYTHON=${BUILD_PYTHON}
	make -C build

clean:
	rm -rf *~ build

bootstrap:
	sh requirements.sh

install:
	make -C build install

uninstall:
	make -C build uninstall
