#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
# CMake configuration file build the Biometric Evaluation Framework and test
# programs. The programs are linked to the shared library built by this
# configuration.
#
# To use CMake:
# Create a build directory; in this example, it is located under the common
# directory, but could be located anywhere:
#     mkdir build; cd build
#     cmake ..
#     make
#
# NOTE: A parallel build (make -j) may fail due to the fact that the test
# programs need to build after the library, but parallel invocation may
# attempt to compile test programs before the library is completely built.
#
# To run a test program:
#     ./bin/test_be_feature_an2kminutiae
#

cmake_minimum_required(VERSION 2.8.11)
project(biomeval)

#
# The CMake config files must exist in the directories added to this config:
#
add_subdirectory(src/libbiomeval/ lib)

#
# Link in the locally built version of the library.
#
if(CMAKE_HOST_APPLE)
  set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-rpath lib")
else(CMAKE_HOST_APPLE)
  set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-Wl,-rpath=lib")
endif(CMAKE_HOST_APPLE)
add_subdirectory(src/test/ bin)
