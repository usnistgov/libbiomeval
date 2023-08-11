#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
# To use CMake to create the Biometric Evaluation Framework library:
#
# Create a build directory; in this example, it will be under libbiomeval:
#     mkdir build; cd build
#     cmake ..
#     make
# To build a static versions of libbiomeval, including the subset of NBIS
# included with the Framework.
#
# Other build scenarios:
# To build the debug version of the library:
#     cmake -DCMAKE_BUILD_TYPE=Debug ..
#
# To use a different compiler for the MPI component (Intel, OpenMPI are among
# the supported compilers):
#     cmake -DMPI_CXX_COMPILER=mpiicpc ..
#
# To build a shared/dynamic library:
#     cmake -DBUILD_SHARED_LIBS=ON ..
#
# To build a static library that also links dependencies statically:
#      cmake -DBUILD_SHARED_LIBS=OFF -DFORCE_STATIC_DEPENDENCIES=ON ..
#
# To build the Framework:
#     make
# To install built library plus headers:
#     make install
# To create an RPM:
#     make package
#

cmake_minimum_required(VERSION 2.8.11)

if (BUILD_FOR_WASM)
	add_definitions("-DWASM")
	if (EMSCRIPTEN)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sUSE_LIBPNG=1 -sUSE_LIBJPEG=1")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sUSE_LIBPNG=1 -sUSE_LIBJPEG=1")

		if (WASM_EXCEPTIONS)
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sSUPPORT_LONGJMP=wasm")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fwasm-exceptions")
			set(LDFLAGS "${LDFLAGS} -fwasm-exceptions")
			set(PNG_NAMES "png-wasm-sjlj")
		endif (WASM_EXCEPTIONS)
	endif (EMSCRIPTEN)

	set(WITH_HWLOC OFF)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" AND FORCE_STATIC_DEPENDENCIES)
	# No static libhwloc in MacPorts
	set(WITH_HWLOC OFF)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" AND FORCE_STATIC_DEPENDENCIES)

if (BUILD_BIOMEVAL_32BIT)
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
endif (BUILD_BIOMEVAL_32BIT)

# Create dirent_windows.h at build time
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../nbis/include/dirent_windows.h")
	if (UNIX)
		execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/../include/be_dirent_windows.h ${CMAKE_CURRENT_SOURCE_DIR}/../../nbis/include/dirent_windows.h)
	elseif(WIN32)
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/../include/be_dirent_windows.h" "${CMAKE_CURRENT_SOURCE_DIR}/../../nbis/include/dirent_windows.h")
	endif()
endif (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../nbis/include/dirent_windows.h")

#
# Get the major and minor library versions from the VERSION file, with format
# expected to be MAJOR_VERSION=mm and MINOR_VERSION=nn on separate lines.
#
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../../VERSION" VERSION_FILE_CONTENTS)
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

#
# Define common options
#
add_definitions(-DMAJOR_VERSION=${MAJOR_VERSION} -DMINOR_VERSION=${MINOR_VERSION})
set(BIOMEVAL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
file(GLOB BIOMEVAL_HEADERS ${BIOMEVAL_INCLUDE}/*.h)
set(NBIS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../../nbis/include)
#
if(${CMAKE_MAJOR_VERSION} GREATER 2)
  cmake_policy(SET CMP0048 NEW)
  project(libbiomeval VERSION ${MAJOR_VERSION}.${MINOR_VERSION})
else()
  project(libbiomeval)
  set(PROJECT_VERSION ${MAJOR_VERSION}.${MINOR_VERSION})
endif()
set(CORELIB biomeval)

# NBIS uses "insecure" functions under Windows
if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS_GLOBALS)
endif(MSVC)

if (BUILD_SHARED_LIBS)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if(${CMAKE_MAJOR_VERSION} GREATER 2)
    if (MSVC)
        set(CMAKE_CXX_STANDARD 17)
    else (MSVC)
        set(CMAKE_CXX_STANDARD 17)
    endif (MSVC)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17")
    endif(MSVC)
endif()

#
# Location of CMake modules local to BIOMEVAL
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/")

# Sanity check our source directory to make sure that we are not trying to
# generate an in-tree build
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed.
CMake would overwrite the makefiles distributed with the Framework.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
Please delete them.")
endif()

set(CORE be_memory_indexedbuffer.cpp be_memory_mutableindexedbuffer.cpp be_text.cpp be_system.cpp be_error.cpp be_error_exception.cpp be_time.cpp be_time_timer.cpp be_time_watchdog.cpp be_error_signal_manager.cpp be_framework.cpp be_framework_status.cpp be_framework_api.cpp be_process_statistics.cpp)

set(IO be_io_properties.cpp be_io_propertiesfile.cpp be_io_utility.cpp be_io_logsheet.cpp be_io_filelogsheet.cpp be_io_syslogsheet.cpp be_io_filelogcabinet.cpp be_io_compressor.cpp be_io_gzip.cpp)

set(RECORDSTORE be_io_recordstore_impl.cpp be_io_recordstore.cpp be_io_dbrecstore.cpp be_io_dbrecstore_impl.cpp be_io_sqliterecstore.cpp be_io_sqliterecstore_impl.cpp be_io_filerecstore.cpp be_io_filerecstore_impl.cpp be_io_listrecstore.cpp be_io_listrecstore_impl.cpp be_io_archiverecstore.cpp be_io_archiverecstore_impl.cpp be_io_compressedrecstore_impl.cpp be_io_compressedrecstore.cpp be_io_recordstoreunion.cpp be_io_recordstoreunion_impl.cpp be_io_persistentrecordstoreunion.cpp be_io_persistentrecordstoreunion_impl.cpp)

set(IMAGE be_image.cpp be_image_image.cpp be_image_jpeg.cpp be_image_jpegl.cpp be_image_netpbm.cpp be_image_raw.cpp be_image_wsq.cpp be_image_png.cpp be_image_jpeg2000.cpp be_image_bmp.cpp be_image_tiff.cpp)

set(FEATURE be_feature.cpp be_feature_minutiae.cpp be_feature_an2k7minutiae.cpp be_feature_incitsminutiae.cpp be_feature_sort.cpp be_feature_an2k11efs.cpp be_feature_an2k11efs_impl.cpp)

set(VIEW be_view_view.cpp be_view_an2kview.cpp be_view_an2kview_varres.cpp)

set(FINGER be_finger.cpp be_finger_an2kminutiae_data_record.cpp be_finger_an2kview.cpp be_finger_an2kview_fixedres.cpp be_latent_an2kview.cpp be_finger_an2kview_capture.cpp be_finger_incitsview.cpp be_finger_ansi2004view.cpp be_finger_ansi2007view.cpp be_finger_iso2005view.cpp be_data_interchange_finger.cpp)

set(PALM be_palm.cpp be_palm_an2kview.cpp)
set(PLANTAR be_plantar.cpp)

set(IRIS be_iris.cpp be_iris_incitsview.cpp be_iris_iso2011view.cpp)
set(FACE be_face.cpp be_face_incitsview.cpp be_face_iso2005view.cpp)

set(DATA be_data_interchange_an2k.cpp be_data_interchange_ansi2004.cpp)

set(PROCESS be_process_worker.cpp be_process_workercontroller.cpp be_process_manager.cpp be_process_forkmanager.cpp be_process_posixthreadmanager.cpp be_process_semaphore.cpp)

set(VIDEO be_video_impl.cpp be_video_container_impl.cpp be_video_stream_impl.cpp be_video_container.cpp be_video_stream.cpp)

set(DEVICE be_device_tlv_impl.cpp be_device_tlv.cpp be_device_smartcard_impl.cpp be_device_smartcard.cpp)

set(MESSAGE_CENTER be_process_messagecenter.cpp be_process_mclistener.cpp be_process_mcreceiver.cpp be_process_mcutility.cpp)

set(MPIBASE be_mpi.cpp be_mpi_csvresources.cpp be_mpi_exception.cpp be_mpi_runtime.cpp be_mpi_workpackage.cpp be_mpi_workpackageprocessor.cpp be_mpi_resources.cpp be_mpi_recordstoreresources.cpp)
set(MPIDISTRIBUTOR be_mpi_distributor.cpp be_mpi_recordstoredistributor.cpp be_mpi_csvdistributor.cpp)
set(MPIRECEIVER be_mpi_receiver.cpp be_mpi_recordprocessor.cpp be_mpi_csvprocessor.cpp)

#
# Some files have not been ported to Windows. Sorry about that.
#
if(MSVC)
    list(REMOVE_ITEM CORE "be_error_signal_manager.cpp" "be_framework_api.cpp" "be_time_watchdog.cpp" "be_process_statistics.cpp")
    list(REMOVE_ITEM IO "be_io_syslogsheet.cpp")

    unset(PROCESS)
    unset(MESSAGE_CENTER)

    list(APPEND CORE "be_sysdeps.cpp")
endif(MSVC)

#
# All the packages for the core library, except:
#	MPI which is built separately and linked in later, optional.
#	VIDEO if the required libraries are present.
#
set(PACKAGES ${CORE} ${IO} ${RECORDSTORE} ${IMAGE} ${FEATURE} ${VIEW} ${DATA} ${FINGER} ${PALM} ${PLANTAR} ${IRIS} ${FACE} ${PROCESS} ${MESSAGE_CENTER})
if (BUILD_FOR_WASM)
	# We're not concerned with these packages for WebAssembly
	list(REMOVE_ITEM PACKAGES ${PROCESS} ${MESSAGE_CENTER})

	# XXX: Remove RecordStore support simply to reduce the number of
	#      dependencies that need to be recompiled for the WASM
	#      architecture. It is likely they can be added back in the future.
	list(REMOVE_ITEM PACKAGES ${RECORDSTORE})
endif (BUILD_FOR_WASM)


if (WITH_FFMPEG)
  # Our FindFFMPEG.cmake is not sufficient for finding all of FFMPEG's
  # dependencies when forcing static dependencies. We must rely on the
  # configuration generated by the FFMPEG build system to determine the entire
  # list of FFMPEG's dependencies, which uses pkg-config.
  if (FORCE_STATIC_DEPENDENCIES)
       if (MSVC)
           # MSVC doesn't have pkg-config, but it does have pkgconf
           set(PKG_CONFIG_EXECUTABLE ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/pkgconf/pkgconf.exe --with-path=${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/pkgconfig)
       endif()
       find_package(PkgConfig REQUIRED)
       pkg_check_modules(FFMPEGPKG REQUIRED
           libavformat
           libavcodec
           libswscale
           libavutil)
  else(FORCE_STATIC_DEPENDENCIES)
    find_package(FFMPEG)
  endif (FORCE_STATIC_DEPENDENCIES)

if(FFMPEG_FOUND OR FFMPEGPKG_FOUND)
  message(STATUS "Adding VIDEO support.")

  add_library(vidobjs OBJECT ${VIDEO})
  target_include_directories(vidobjs PUBLIC ${BIOMEVAL_INCLUDE})
  include_directories(PUBLIC ${FFMPEG_INCLUDE_DIR})
else(FFMPEG_FOUND OR FFMPEGPKG_FOUND)
  # FIXME: Should we complain if WITH_FFMPEG=YES but FFMPEG is not found?
  message(STATUS "Building without VIDEO support.")
endif(FFMPEG_FOUND OR FFMPEGPKG_FOUND)
else(WITH_FFMPEG)
  message(STATUS "Building without VIDEO support.")
endif(WITH_FFMPEG)

#
# PCSC (Smartcard) support is an optional library
#
if (WITH_PCSC)
if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	find_library(PCSC_FRAMEWORK PCSC)
	if (PCSC_FRAMEWORK)
		set(PCSC_FOUND ON)
	endif (PCSC_FRAMEWORK)
else (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	find_package(PCSC)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")

if (PCSC_FOUND)
	message(STATUS "Adding PCSC support.")
	add_library(deviceobjs OBJECT ${DEVICE})
	target_include_directories(deviceobjs PUBLIC ${BIOMEVAL_INCLUDE})
else (PCSC_FOUND)
	# FIXME: Should we complain if WITH_PCSC=YES but PCSC is not found?
	message(STATUS "Building without PCSC support.")
endif (PCSC_FOUND)
else(WITH_PCSC)
	message(STATUS "Building without PCSC support.")
endif(WITH_PCSC)

#
# Keep MPI related files separate so we can use a different compiler command.
# MPI files are built as an object-only lib (not linked) so its symbols can
# be added to the BIOMEVAL library later. MPI is optional.
#
set(MPISOURCES ${MPIBASE} ${MPIDISTRIBUTOR} ${MPIRECEIVER})
if(WITH_MPI)
# Provide some hints to help find MPI
list(APPEND CMAKE_PREFIX_PATH "/opt/local/bin")
list(APPEND CMAKE_PREFIX_PATH "/usr/lib/openmpi/bin")
list(APPEND CMAKE_PREFIX_PATH "/usr/lib64/openmpi/bin")
find_package(MPI)
if(MPI_CXX_FOUND AND NOT MSVC)
  add_library(mpiobjs OBJECT ${MPISOURCES})
  if (${CMAKE_VERSION} VERSION_LESS "3.10")
    set_target_properties(
      mpiobjs
      PROPERTIES COMPILE_FLAGS "${MPI_CXX_COMPILE_FLAGS}")
    target_include_directories(
      mpiobjs
      PUBLIC ${MPI_CXX_INCLUDE_PATH} ${BIOMEVAL_INCLUDE})
  else(${CMAKE_VERSION} VERSION_LESS "3.10")
    target_compile_definitions(mpiobjs PUBLIC "${MPI_CXX_COMPILE_DEFINITIONS}")
    target_compile_options(mpiobjs PUBLIC "${MPI_CXX_COMPILE_OPTIONS}")
    target_include_directories(
      mpiobjs
      PUBLIC ${MPI_CXX_INCLUDE_DIRS} ${BIOMEVAL_INCLUDE})
  endif(${CMAKE_VERSION} VERSION_LESS "3.10")
else(MPI_CXX_FOUND AND NOT MSVC)
  # FIXME: Should we stop if WITH_MPI=ON, but MPI not found?
  message(STATUS "Building without MPI support.")
endif(MPI_CXX_FOUND AND NOT MSVC)
else(WITH_MPI)
  message(STATUS "Building without MPI support.")
endif(WITH_MPI)

#
# Build the Framework-local version of NBIS
#
file(GLOB_RECURSE NBISSOURCE "${PROJECT_SOURCE_DIR}/../../nbis/lib/*.c")
add_library(nbisobjs OBJECT ${NBISSOURCE})
target_include_directories(nbisobjs PUBLIC ${NBIS_INCLUDE})

#
# Big-Endian check needed for NBIS
#
include(TestBigEndian)
TEST_BIG_ENDIAN(BIGEND)
if(NOT ${BIGEND})
  add_definitions("-D__NBISLE__")
endif(NOT ${BIGEND})

#
# Build the core library by building the .cpp
# files into object-only libraries, then combining the object files together
# with the BIOMEVAL and NBIS objects.
#
set(OBJLIBS "")

add_library(coreobjs OBJECT ${PACKAGES})

list(APPEND OBJLIBS $<TARGET_OBJECTS:coreobjs>)
list(APPEND OBJLIBS $<TARGET_OBJECTS:nbisobjs>)
if(MPI_CXX_FOUND AND NOT MSVC)
  list(APPEND OBJLIBS $<TARGET_OBJECTS:mpiobjs>)
endif(MPI_CXX_FOUND AND NOT MSVC)
if(FFMPEG_FOUND OR FFMPEGPKG_FOUND)
  list(APPEND OBJLIBS $<TARGET_OBJECTS:vidobjs>)
endif(FFMPEG_FOUND OR FFMPEGPKG_FOUND)
if(PCSC_FOUND)
  list(APPEND OBJLIBS $<TARGET_OBJECTS:deviceobjs>)
endif(PCSC_FOUND)

# "Some native build systems (such as Xcode) may not like targets that have
#  only object files, so consider adding at least one real source file to any
#  target that references $<TARGET_OBJECTS:objlib>".
# https://cmake.org/cmake/help/v3.12/command/add_library.html#object-libraries
if(CMAKE_GENERATOR STREQUAL Xcode)
	set(XCODE_GENERATOR_FIX "be_fix_cmake_xcode_generator.cpp")
	file(WRITE "${XCODE_GENERATOR_FIX}" "")
	list(APPEND OBJLIBS "${XCODE_GENERATOR_FIX}")
endif(CMAKE_GENERATOR STREQUAL Xcode)

if (BUILD_SHARED_LIBS)
	add_library(${CORELIB} SHARED ${OBJLIBS})
	set_target_properties(${CORELIB} PROPERTIES
            POSITION_INDEPENDENT_CODE ON
            WINDOWS_EXPORT_ALL_SYMBOLS ON)
else()
	add_library(${CORELIB} STATIC ${OBJLIBS})
	set_target_properties(${CORELIB} PROPERTIES
            POSITION_INDEPENDENT_CODE OFF)
endif(BUILD_SHARED_LIBS)

set_target_properties(
    ${CORELIB}
    PROPERTIES VERSION ${PROJECT_VERSION}
    SOVERSION ${MAJOR_VERSION}
    PUBLIC_HEADER "${BIOMEVAL_HEADERS}")

if (FFMPEG_FOUND OR FFMPEGPKG_FOUND)
  if (FORCE_STATIC_DEPENDENCIES)
       target_link_libraries(${CORELIB} ${FFMPEGPKG_STATIC_LINK_LIBRARIES})
  else (FORCE_STATIC_DEPENDENCIES)
      target_link_libraries(${CORELIB} ${FFMPEG_LIBRARIES})
  endif (FORCE_STATIC_DEPENDENCIES)
endif (FFMPEG_FOUND OR FFMPEGPKG_FOUND)


# Windows needs to differentiate between release/debug builds.
if (MSVC)
	if (CMAKE_VERSION VERSION_GREATER 3.14.9999)
		set_target_properties(${CORELIB} PROPERTIES
		    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<BOOL:${BUILD_SHARED_LIBS}>:DLL>")
		set_target_properties(coreobjs PROPERTIES
		    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<BOOL:${BUILD_SHARED_LIBS}>:DLL>")
		set_target_properties(nbisobjs PROPERTIES
		    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<BOOL:${BUILD_SHARED_LIBS}>:DLL>")

		if (FFMPEG_FOUND OR FFMPEGPKG_FOUND)
			set_target_properties(vidobjs PROPERTIES
			    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<BOOL:${BUILD_SHARED_LIBS}>:DLL>")
		endif (FFMPEG_FOUND OR FFMPEGPKG_FOUND)

		if (PCSC_FOUND)
			set_target_properties(deviceobjs PROPERTIES
			    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<BOOL:${BUILD_SHARED_LIBS}>:DLL>")
		endif (PCSC_FOUND)
	elseif (CMAKE_VERSION VERSION_GREATER 3.14.9999)
		set_target_properties(
		    ${CORELIB} nbisobjs coreobjs vidobjs deviceobjs
		    PROPERTIES
		    CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd"
		    CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")

		if (FFMPEG_FOUND OR FFMPEGPKG_FOUND)
			set_target_properties(vidobjs
			    PROPERTIES
			    CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd"
			    CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
		endif (FFMPEG_FOUND OR FFMPEGPKG_FOUND)

		if (PCSC_FOUND)
			set_target_properties(deviceobjs
			    PROPERTIES
			    CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd"
			    CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
		endif (PCSC_FOUND)
	endif (CMAKE_VERSION VERSION_GREATER 3.14.9999)
endif(MSVC)

if (MSVC)
	set(CMAKE_DEBUG_POSTFIX "d")
endif (MSVC)

#
# For OS-X, add paths where the ports packages are placed.
#
if(CMAKE_HOST_APPLE)
  if (NOT BUILD_FOR_WASM)
  target_include_directories(coreobjs PUBLIC /opt/local/include)
  set_target_properties(${CORELIB} PROPERTIES MACOSX_RPATH ON)
  target_link_libraries(${CORELIB} -L/opt/local/lib)
  endif (NOT BUILD_FOR_WASM)
  if (PCSC_FOUND)
	target_link_libraries(${CORELIB} ${PCSC_FRAMEWORK})
  endif (PCSC_FOUND)
  if (NOT BUILD_FOR_WASM)
  add_definitions("-DDarwin")
  endif(NOT BUILD_FOR_WASM)
endif(CMAKE_HOST_APPLE)

target_include_directories(coreobjs PUBLIC ${BIOMEVAL_INCLUDE} ${NBIS_INCLUDE})

#
# Add the definition for Linux that is used as a conditional within a
# few source code files.
#
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  message(STATUS ">>> Linux")
  add_definitions("-DLinux")
endif()

#
# OpenSSL
#
# On macOS, we use the built-in Security framework instead of OpenSSL, but the
# Security framework requires C "block" support, a language extension not found
# in all compilers.
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fblocks" BLOCK_SUPPORT)
if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" AND BLOCK_SUPPORT)
	find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
	find_library(SECURITY_FRAMEWORK Security REQUIRED)
	target_link_libraries(${CORELIB}
	    ${FOUNDATION_FRAMEWORK} ${SECURITY_FRAMEWORK})
	add_definitions("-DUseAppleSecurityFramework")
	message(STATUS "Using Apple Security Framework")
else (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" AND BLOCK_SUPPORT)
	set(OPENSSL_FIND_REQUIRED TRUE)
	find_package(OpenSSL)
	if(NOT OPENSSL_FOUND)
	  message(FATAL_ERROR "Could not find OpenSSL.")
	endif(NOT OPENSSL_FOUND)
	include_directories(PUBLIC ${OPENSSL_INCLUDE_DIR})
	target_link_libraries(${CORELIB} ${OPENSSL_LIBRARIES})
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" AND BLOCK_SUPPORT)

#
# Berkeley DB
#
if (NOT BUILD_FOR_WASM)
set(BERKELEYDB_FIND_REQUIRED TRUE)
find_package(BERKELEYDB)
if(NOT BERKELEYDB_FOUND)
  message(FATAL_ERROR "Could not find Berkeley Database.")
endif(NOT BERKELEYDB_FOUND)
include_directories(PUBLIC ${BERKELEYDB_INCLUDE_DIR})
target_link_libraries(${CORELIB} ${BERKELEYDB_LIBRARIES})
endif()

#
# OpenJPEG and JPEG2000
#
set(OPENJP2_FIND_REQUIRED TRUE)
find_package(OPENJP2)
if(NOT OPENJP2_FOUND)
  message(FATAL_ERROR "Could not find OpenJP2.")
endif(NOT OPENJP2_FOUND)
include_directories(PUBLIC ${OPENJP2_INCLUDE_DIR})
target_link_libraries(${CORELIB} ${OPENJP2_LIBRARIES})

set(JPEG_FIND_REQUIRED TRUE)
find_package(JPEG)
if(NOT JPEG_FOUND)
  message(FATAL_ERROR "Could not find JPEG.")
endif(NOT JPEG_FOUND)
include_directories(PUBLIC ${JPEG_INCLUDE_DIR})
target_link_libraries(${CORELIB} ${JPEG_LIBRARIES})

#
# OpenMPI
#
if(MPI_CXX_FOUND AND NOT MSVC)
  message(STATUS "Adding MPI support to ${CORELIB}.")
  target_link_libraries(${CORELIB} ${MPI_CXX_LINK_FLAGS} ${MPI_CXX_LIBRARIES})
else(MPI_CXX_FOUND AND NOT MSVC)
  message(STATUS "Will build ${CORELIB} without MPI support.")
endif(MPI_CXX_FOUND AND NOT MSVC)

#
# PCSC
#
if(PCSC_FOUND)
   include_directories(PUBLIC ${PCSC_INCLUDE_DIR})
   target_include_directories(deviceobjs PUBLIC ${BIOMEVAL_INCLUDE})
   target_link_libraries(${CORELIB} ${PCSC_LIBRARIES})
endif(PCSC_FOUND)

#
# The Portable Hardware Locality hwloc
#
if (WITH_HWLOC)
add_definitions(-DBIOMEVAL_WITH_HWLOC)
set(HWLOC_FIND_REQUIRED TRUE)
find_package(HWLOC)
if(NOT HWLOC_FOUND)
  message(FATAL_ERROR "Could not find hwloc.")
endif(NOT HWLOC_FOUND)
include_directories(PUBLIC ${HWLOC_INCLUDE_DIR})
target_link_libraries(${CORELIB} ${HWLOC_LIBRARIES})
else (WITH_HWLOC)
    message(STATUS "Building without HWLOC support.")
endif (WITH_HWLOC)

#
# Other libs not specifically searched for above.
#

# GCC < 9 needs to explicitly link libstdc++fs
target_link_libraries(${CORELIB}
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)

if (NOT BUILD_FOR_WASM)
# This will use our Module
find_package(SQLITE3 REQUIRED)
target_link_libraries(${CORELIB} ${SQLITE3_LIBRARIES})
endif()

# libtiff, like ffmpeg, writes a pkg-config, but it prefers to link against
# shared libs. We can't have that with this option.
find_package(TIFF REQUIRED)
target_link_libraries(${CORELIB} ${TIFF_LIBRARIES})
if (FORCE_STATIC_DEPENDENCIES)
    find_library(ZSTD_LIBRARY zstd REQUIRED)
    find_library(LZMA_LIBRARY lzma REQUIRED)

    # libtiff on MacPorts is now compiled to support LERC compression,
    # requiring libLerc when linked statically. MacPorts does not build libLerc
    # statically, so you need to build it yourself with -DBUILD_SHARED_LIBS=OFF.
    find_file(TIFF_CONFIG_HEADER
        NAMES tiffconf.h
        REQUIRED
        HINTS ${TIFF_INCLUDE_DIR})
    file(READ "${TIFF_CONFIG_HEADER}" TIFF_CONFIG)
    string(REGEX MATCH "#define LERC_SUPPORT 1" TIFF_REQUIRES_LERC "${TIFF_CONFIG}")
    if ("${TIFF_REQUIRES_LERC}" STREQUAL "")
            target_link_libraries(${CORELIB} ${ZSTD_LIBRARY} ${LZMA_LIBRARY})
    else()
    	find_library(LERC_LIBRARY Lerc REQUIRED)
        target_link_libraries(${CORELIB} ${ZSTD_LIBRARY} ${LZMA_LIBRARY} ${LERC_LIBRARY})
    endif()
endif (FORCE_STATIC_DEPENDENCIES)

find_package(PNG REQUIRED)
target_link_libraries(${CORELIB} ${PNG_LIBRARIES})

# Windows-specific system libraries
if(MSVC)
	# Need Winsock on Windows
	target_link_libraries(${CORELIB} ws2_32)

	# Need crypt32 on static Win32 builds
	if (NOT BUILD_SHARED_LIBS)
		target_link_libraries(${CORELIB} crypt32)
	endif (NOT BUILD_SHARED_LIBS)
endif(MSVC)


target_include_directories(${CORELIB} PUBLIC $<INSTALL_INTERFACE:include>)

#
# Installation.
#
include(GNUInstallDirs)
install(
  TARGETS ${CORELIB}
  EXPORT biomeval-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/biomevalConfig.cmake.in
    ${CMAKE_BINARY_DIR}/cmake/biomeval-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/biomeval
)

write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/cmake/biomeval-config-version.cmake
    VERSION ${biomeval_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    FILES
        ${CMAKE_BINARY_DIR}/cmake/biomeval-config.cmake
        ${CMAKE_BINARY_DIR}/cmake/biomeval-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/biomeval
)

# Install dependent vcpkg DLLs
if (MSVC)
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}$<$<CONFIG:Debug>:/Debug>$<$<CONFIG:Release>:/Release>/
	    DESTINATION ${CMAKE_INSTALL_BINDIR}
	    FILES_MATCHING
	    PATTERN "*dll"
	    PATTERN "${CORELIB}.dll" EXCLUDE)
endif (MSVC)

install(
    DIRECTORY
        "${PROJECT_SOURCE_DIR}/cmake/Modules/"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/biomeval/Modules
)

install(EXPORT biomeval-targets
    FILE biomeval-targets.cmake
    NAMESPACE biomeval::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/biomeval
)

export(EXPORT biomeval-targets
    FILE ${CMAKE_BINARY_DIR}/cmake/biomeval-targets.cmake
    NAMESPACE biomeval::
)


#
# RPM Creation
#
# Ignore the system directories as they are not owned by this project.
# This technique is borrowed from https://gitorious.org/brewtarget/brewtarget/
#
SET(CPACK_RPM_SPEC_MORE_DEFINE "%define ignore \#")
SET(CPACK_RPM_USER_FILELIST
  "%ignore /usr"
  "%ignore /usr/local"
  "%ignore /usr/local/lib"
  "%ignore /usr/local/include")

set(CPACK_RPM_PACKAGE_AUTOREQ no)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
if (NOT BUILD_SHARED_LIBS)
	string(APPEND CPACK_PACKAGE_NAME "-static")
endif(NOT BUILD_SHARED_LIBS)
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "NIST Image Group (www.nist.gov/itl/iad/image-group)")
set(CPACK_PACKAGE_VENDOR "NIST (www.nist.gov)")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)
