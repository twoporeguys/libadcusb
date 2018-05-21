.PHONY: all

export CC := clang
export CXX := clang++
BUILD_PYTHON := ON
PYTHON_VERSION ?= "python3"
INSTALL_PREFIX ?= "/usr/local"

all:
	mkdir -p build && \
	cd build && \
	cmake .. -DPYTHON_VERSION=${PYTHON_VERSION} \
	-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} \
	-DBUILD_PYTHON=${BUILD_PYTHON} && \
	make

.PHONY: clean
clean:
	rm -rf *~ build

.PHONY: bootstrap
bootstrap:
	sh requirements.sh

PREFIX = /usr/local

.PHONY: install
install:
	make -C build install

.PHONY: uninstall
uninstall:
	make -C build uninstall
