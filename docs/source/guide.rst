User's guide
============

Building and installing
-----------------------
`make` and `make install` should work as expected. In order to customize
build options (see below), you need to call `cmake <http://cmake.org>`_
directly inside source tree.

Build options
^^^^^^^^^^^^^
+------------------------+---------+---------------------------------------+
| Option name            | Default | Description                           |
+========================+=========+=======================================+
| BUNDLED_BLOCKS_RUNTIME | OFF     | Use bundled BlocksRuntime library     |
|                        |         | instead of the system one             |
+------------------------+---------+---------------------------------------+
| BUILD_DOC              | OFF     | Build documentation                   |
+------------------------+---------+---------------------------------------+
| PYTHON_VERSION         | 2       | Python version to create bindings for |
+------------------------+---------+---------------------------------------+

What libadcusb is and what it's not
-----------------------------------
`libadcusb <https://github.com/twoporeguys/libadcusb>`_ is a library designed
for reading ADC acquisition data over the isochronous data endpoint defined
in `mom-firmware <https://github.com/twoporeguys/mom-firmware>`_. It's not
meant to provide device enumeration, ability to send any requests to the
device or handle hot-plug events. You need
`librpc <https://github.com/twoporeguys/librpc>`_ to handle that stuff.

Buffering
---------

Reading acquisition data
------------------------
