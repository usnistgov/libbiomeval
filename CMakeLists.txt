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

cmake_minimum_required(VERSION 3.20)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VERSION_FILE_CONTENTS)
string(REGEX REPLACE "\n" ";" VERSION_FILE_CONTENTS ${VERSION_FILE_CONTENTS})
foreach (line ${VERSION_FILE_CONTENTS})
	string(REGEX MATCH "MAJOR_VERSION=.+" MAJOR_VERSION_FOUND "${line}")
	if (MAJOR_VERSION_FOUND)
		string(REPLACE "MAJOR_VERSION=" "" MAJOR_VERSION "${line}")
	endif ()

	string(REGEX MATCH "MINOR_VERSION=.+" MINOR_VERSION_FOUND "${line}")
	if (MINOR_VERSION_FOUND)
		string(REPLACE "MINOR_VERSION=" "" MINOR_VERSION "${line}")
	endif ()

	string(REGEX MATCH "COMPATIBILITY_VERSION=.+" COMPATIBILITY_VERSION_FOUND "${line}")
	if (COMPATIBILITY_VERSION_FOUND)
		string(REPLACE "COMPATIBILITY_VERSION=" "" COMPATIBILITY_VERSION "${line}")
	endif ()
endforeach ()

project(biomeval VERSION ${MAJOR_VERSION}.${MINOR_VERSION})

if (BUILD_SHARED_LIBS)
	message(STATUS "Build type: SHARED")
else()
	message(STATUS "Build type: STATIC")
endif()

include(CMakeDependentOption)

# 32-bit compilation on systems that support compiling both 32- and 64-bit. On
# Windows, this is part of the platform (cmake's -A flag).

# Detect if it seems we're on a 64-bit machine capable of also producing a
# 32-bit build.
if ((NOT MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8) OR (CMAKE_GENERATOR STREQUAL "Xcode" AND XCODE_VERSION VERSION_LESS 10))
	set(POSSIBLE_TO_BUILD_32BIT YES)
else ()
	set(POSSIBLE_TO_BUILD_32BIT NO)
endif ()

if (POSSIBLE_TO_BUILD_32BIT)
	option(BUILD_BIOMEVAL_32BIT "Build code for 32-bit architecture." OFF)
endif ()
if (NOT MSVC AND POSSIBLE_TO_BUILD_32BIT)
	if (BUILD_BIOMEVAL_32BIT)
		message(STATUS "Bitness: 32-bit")
	else (BUILD_BIOMEVAL_32BIT)
		message(STATUS "Bitness: 64-bit")
	endif (BUILD_BIOMEVAL_32BIT)
endif ()

# Build sources that require libhwloc
option(WITH_HWLOC "Build sources that require libhwloc" ON)
# Build sources that require OpenMPI
option(WITH_MPI "Build sources that require MPI" ON)
# Build sources that require PCSC
option(WITH_PCSC "Build sources that require PCSC" ON)
# Disable things that aren't well supported under WASM
option(BUILD_FOR_WASM "Build in a way that supports WASM" OFF)
# Auto-enable WASM build if we can detect emscripten
if (BUILD_FOR_WASM)
	message(STATUS "Disabling unsupported components to enable WASM build")
elseif(EMSCRIPTEN)
	message(STATUS "Emscripten detected, enabling WASM by disabling unsupported components")
	set(BUILD_FOR_WASM ON)
endif(BUILD_FOR_WASM)
# Use WASM exceptions (over JavaScript exceptions)
cmake_dependent_option(WASM_EXCEPTIONS "Use WASM exceptions (instead of JavaScript exceptions)" ON "BUILD_FOR_WASM;EMSCRIPTEN" OFF)
if (WASM_EXCEPTIONS)
	message(STATUS "Enabling WebAssembly exception handling")
endif (WASM_EXCEPTIONS)

# Build sources that require FFMPEG.
option(WITH_FFMPEG "Build sources that require FFMPEG" ON)

#
# The CMake config files must exist in the directories added to this config:
#
add_subdirectory(src/libbiomeval/ lib)

if (BUILD_BIOMEVAL_TESTS)
	message(STATUS "Test executables: ON")
else ()
	message(STATUS "Test executables: OFF")
endif ()

if (BUILD_BIOMEVAL_TESTS)
	add_subdirectory(src/test/ bin)
endif ()
