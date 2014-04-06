cython-cmake-example
====================


Introduction
------------

Cython_ is hybrid C-Python language to easily write Python C-extensions.  Cython
allows one to write fast Python-ish code and easily integrate C or C++ code.

CMake_ is a cross-platform build system with good support for C/C++.  With
simple projects description scripts, *CMakeLists.txt*, a powerful configuration
system is available.  The configuration can be handled with a command line
interface, curses interface, or Qt GUI interface.

This repository has CMake utilities for building Cython projects with CMake.
The rectangle example from the Cython documentation is built to demonstrate
their use.  An example is also provided that shows registering a C callback
with a function defined in a Cython module.

The features of this build system include:

- Easy configuration of build settings.
- Easy integration of external libraries.
- Implicit Makefile dependency generation for .pxd files.
- Implicit Makefile dependency generation for C/C++ headers.
- Out-of-source builds.

The project is tested across platforms on the `nightly dashboard`_.

Dependencies
------------

Build Dependencies
^^^^^^^^^^^^^^^^^^

- Python_ (also works with CMakeified-Python_)
- Cython_
- CMake_
- C++ compiler (g++ for instance)

Test Dependencies
^^^^^^^^^^^^^^^^^

- Nose_


Build Instructions
------------------

::

  mkdir cython_example_build
  cd cython_example_build
  cmake /path/to/src/cython-cmake-example
  make

.. warning::

  In your CMake configuration, make sure that PYTHON_LIBRARY,
  PYTHON_INCLUDE_DIR, and CYTHON_EXECUTABLE are all using the same CPython
  version.

To run the tests::

  nosetests

.. _Cython: http://cython.org/
.. _CMake:  http://cmake.org/
.. _Nose:   http://pypi.python.org/pypi/nose/
.. _Python: http://python.org/
.. _nightly dashboard: http://my.cdash.org/index.php?project=cython-cmake-example
.. _CMakeified-Python: https://github.com/davidsansome/python-cmake-buildsystem
