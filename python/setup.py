#
# Copyright 2017 Two Pore Guys, Inc.
# All rights reserved
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted providing that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

import os
import numpy as np
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
            extra_compile_args=["-fblocks", "-Wno-sometimes-uninitialized"],
            extra_link_args=["-g", "-ladcusb"],
            include_dirs=[np.get_include()]
        )
    ]
)
