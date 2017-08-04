#
# Copyright 2017 Two Pore Guys, Inc.
# All rights reserved.
#
# This is proprietary software.
#

import os
import Cython.Compiler.Options
Cython.Compiler.Options.annotate = True

from distutils.core import setup
from Cython.Distutils.extension import Extension
from Cython.Distutils import build_ext

os.environ['CC'] = 'clang'
os.environ.setdefault('DESTDIR', '/')

setup(
    name='adcusb',
    version='1.0',
    packages=[''],
    package_data={'': ['*.html', '*.c', 'adcusb.pxd']},
    cmdclass={'build_ext': build_ext},
    ext_modules=[
        Extension(
            "adcusb",
            ["adcusb.pyx"],
            extra_compile_args=["-fblocks"],
            extra_link_args=["-g", "-ladcusb"],
        )
    ]
)
