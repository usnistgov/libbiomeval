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
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VERSION_FILE_CONTENTS)
string(REGEX REPLACE "\n" ";" VERSION_FILE_CONTENTS ${VERSION_FILE_CONTENTS})
foreach (line ${VERSION_FILE_CONTENTS})
	string(REGEX MATCH "MAJOR_VERSION=.+" MAJOR_VERSION_FOUND "${line}")
	if (MAJOR_VERSION_FOUND)
		string(REPLACE "MAJOR_VERSION=" "" MAJOR_VERSION "${line}")
	endif (MAJOR_VERSION_FOUND)

	string(REGEX MATCH "MINOR_VERSION=.+" MINOR_VERSION_FOUND "${line}")
	if (MINOR_VERSION_FOUND)
		string(REPLACE "MINOR_VERSION=" "" MINOR_VERSION "${line}")
	endif (MINOR_VERSION_FOUND)

	string(REGEX MATCH "COMPATIBILITY_VERSION=.+" COMPATIBILITY_VERSION_FOUND "${line}")
	if (COMPATIBILITY_VERSION_FOUND)
		string(REPLACE "COMPATIBILITY_VERSION=" "" COMPATIBILITY_VERSION "${line}")
	endif (COMPATIBILITY_VERSION_FOUND)
endforeach (line ${VERSION_FILE_CONTENTS})

# Don't use CXX_FLAGS for MSVC runtime lib
if (CMAKE_VERSION VERSION_GREATER 3.14.999)
	cmake_policy(SET CMP0091 NEW)
endif (CMAKE_VERSION VERSION_GREATER 3.14.999)

if(${CMAKE_MAJOR_VERSION} VERSION_GREATER 2)
  cmake_policy(SET CMP0048 NEW)
  project(biomeval VERSION ${MAJOR_VERSION}.${MINOR_VERSION})
else()
  project(biomeval)
  set(PROJECT_VERSION ${MAJOR_VERSION}.${MINOR_VERSION})
endif()

if (BUILD_SHARED_LIBS)
	message(STATUS "Build type: SHARED")
else()
	message(STATUS "Build type: STATIC")
endif(BUILD_SHARED_LIBS)

include(CMakeDependentOption)
cmake_dependent_option(FORCE_STATIC_DEPENDENCIES "Force linking against static versions of dependencies" OFF "NOT MSVC" OFF)
if (NOT MSVC)
	if (FORCE_STATIC_DEPENDENCIES)
		set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
		message(STATUS "Dependency type: STATIC only")
	else (FORCE_STATIC_DEPENDENCIES)
		message(STATUS "Dependency type: SHARED preferred, STATIC possible")
	endif (FORCE_STATIC_DEPENDENCIES)
endif(NOT MSVC)

# 32-bit compilation on systems that support compiling both 32- and 64-bit. On
# Windows, this is part of the platform (cmake's -A flag).

# Detect if it seems we're on a 64-bit machine capable of also producing a
# 32-bit build.
if ((NOT MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8) OR (CMAKE_GENERATOR STREQUAL "Xcode" AND XCODE_VERSION VERSION_LESS 10))
	set(POSSIBLE_TO_BUILD_32BIT YES)
else ((NOT MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8) OR (CMAKE_GENERATOR STREQUAL "Xcode" AND XCODE_VERSION VERSION_LESS 10))
	set(POSSIBLE_TO_BUILD_32BIT NO)
endif ((NOT MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8) OR (CMAKE_GENERATOR STREQUAL "Xcode" AND XCODE_VERSION VERSION_LESS 10))

if (POSSIBLE_TO_BUILD_32BIT)
	option(BUILD_BIOMEVAL_32BIT "Build code for 32-bit architecture." OFF)
endif (POSSIBLE_TO_BUILD_32BIT)
if (NOT MSVC AND POSSIBLE_TO_BUILD_32BIT)
	if (BUILD_BIOMEVAL_32BIT)
		message(STATUS "Bitness: 32-bit")
	else (BUILD_BIOMEVAL_32BIT)
		message(STATUS "Bitness: 64-bit")
	endif (BUILD_BIOMEVAL_32BIT)
endif (NOT MSVC AND POSSIBLE_TO_BUILD_32BIT)

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

# Build sources that require FFMPEG. FFMPEG has *many* dynamic dependencies that
# aren't built static on all systems, therefore we cannot guarantee they'll be
# found.
cmake_dependent_option(WITH_FFMPEG "Build sources that require FFMPEG" ON "NOT FORCE_STATIC_DEPENDENCIES" OFF)

#
# The CMake config files must exist in the directories added to this config:
#
add_subdirectory(src/libbiomeval/ lib)

#
# Link in the locally built version of the library.
#
if(CMAKE_HOST_APPLE)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rpath lib")
else(CMAKE_HOST_APPLE)
  if(NOT MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath=lib")
  endif(NOT MSVC)
endif(CMAKE_HOST_APPLE)

if (BUILD_BIOMEVAL_TESTS)
	message(STATUS "Test executables: ON")
else (BUILD_BIOMEVAL_TESTS)
	message(STATUS "Test executables: OFF")
endif (BUILD_BIOMEVAL_TESTS)

if (BUILD_BIOMEVAL_TESTS)
	add_subdirectory(src/test/ bin)
endif (BUILD_BIOMEVAL_TESTS)
