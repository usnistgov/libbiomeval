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
# To build static and shared library versions of libbiomeval, including the
# subset of NBIS included with the Framework.
#
# Other build scenarios:
# To build the debug version of the library:
#     cmake -DCMAKE_BUILD_TYPE=Debug ..
#
# To use a different compiler for the MPI component (Intel, OpenMPI are among
# the supported compilers):
#     cmake -DMPI_CXX_COMPILER=mpiicpc ..
#
# Get the major and minor library versions from the VERSION file, with format
# expected to be MAJOR_VERSION=mm and MINOR_VERSION=nn on separate lines.
#
execute_process(COMMAND grep MAJOR_VERSION= ${CMAKE_SOURCE_DIR}/../../VERSION COMMAND cut -f2 -d= OUTPUT_VARIABLE MAJOR_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND grep MINOR_VERSION= ${CMAKE_SOURCE_DIR}/../../VERSION COMMAND cut -f2 -d= OUTPUT_VARIABLE MINOR_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND grep COMPATIBILITY_VERSION= ${CMAKE_SOURCE_DIR}/../../VERSION COMMAND cut -f2 -d= OUTPUT_VARIABLE COMPATIBILITY_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)

#
# Define common options, including that for Berkeley DB version < 2
#
add_definitions(-DMAJOR_VERSION -DMINOR_VERSION -DDB1X)

#
project(BIOMEVAL)
set(CORELIB biomeval)
set(SHAREDLIB ${CORELIB})
#
# The library names. Because a target name cannot be duplicated, build the
# shared and static libraries under two different names, but the output
# filename for the static will be the different than the target name.
set(STATICLIB ${CORELIB})
set(STATICLIBTGT ${CORELIB}-static)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")

#
# Location of CMake modules local to BIOMEVAL
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

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

cmake_minimum_required(VERSION 2.8.11)

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
# All the packages for the core library, except:
#	MPI which is built separately and linked in later, optional.
#	VIDEO if the required libraries are present.
#
set(PACKAGES ${CORE} ${IO} ${RECORDSTORE} ${IMAGE} ${FEATURE} ${VIEW} ${DATA} ${FINGER} ${PALM} ${PLANTAR} ${IRIS} ${FACE} ${PROCESS} ${DEVICE} ${MESSAGE_CENTER})

find_package(FFMPEG)
if(FFMPEG_FOUND)
  set(PACKAGES ${PACKAGES} ${VIDEO})
  message(STATUS "Adding VIDEO support.")
else(FFMPEG_FOUND)
  message(STATUS "Building without VIDEO support.")
endif(FFMPEG_FOUND)

#
# Keep MPI related files separate so we can use a different compiler command.
# MPI files are built as an object-only lib (not linked) so its symbols can
# be added to the BIOMEVAL library later. MPI is optional.
#
set(MPISOURCES ${MPIBASE} ${MPIDISTRIBUTOR} ${MPIRECEIVER})
find_package(MPI)
if(MPI_CXX_FOUND)
  add_library(mpiobjs OBJECT ${MPISOURCES})
  set_target_properties(mpiobjs PROPERTIES CXX_COMPILER_LAUNCHER "${MPI_CXX_COMPILER}")
  set_target_properties(mpiobjs PROPERTIES COMPILE_FLAGS "${MPI_CXX_COMPILE_FLAGS}")
  target_include_directories(mpiobjs PUBLIC ${MPI_CXX_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/../include)
endif(MPI_CXX_FOUND)

#
# Build the Framework-local version of NBIS
#
file(GLOB_RECURSE NBISSOURCE "${CMAKE_SOURCE_DIR}/../../nbis/lib/*.c")
add_library(nbisobjs OBJECT ${NBISSOURCE})
target_include_directories(nbisobjs PUBLIC ${CMAKE_SOURCE_DIR}/../../nbis/include)

#
# Big-Endian check needed for NBIS
#
include(TestBigEndian)
TEST_BIG_ENDIAN(BIGEND)
if(NOT ${BIGEND})
  add_definitions("-D__NBISLE__")
endif(NOT ${BIGEND})

#
# Build the core library, shared and static versions, by building the .cpp
# files into object-only libraries, then combining the object files together
# with the MPI and NBIS objects.
#
add_library(coreobjs OBJECT ${PACKAGES})
if(MPI_CXX_FOUND)
  add_library(${SHAREDLIB} SHARED $<TARGET_OBJECTS:coreobjs> $<TARGET_OBJECTS:mpiobjs> $<TARGET_OBJECTS:nbisobjs>)
  add_library(${STATICLIBTGT} STATIC $<TARGET_OBJECTS:coreobjs> $<TARGET_OBJECTS:mpiobjs> $<TARGET_OBJECTS:nbisobjs>)
else(MPI_CXX_FOUND)
  add_library(${SHAREDLIB} SHARED $<TARGET_OBJECTS:coreobjs> $<TARGET_OBJECTS:nbisobjs>)
  add_library(${STATICLIBTGT} STATIC $<TARGET_OBJECTS:coreobjs> $<TARGET_OBJECTS:nbisobjs>)
endif(MPI_CXX_FOUND)
set_target_properties(${SHAREDLIB} PROPERTIES VERSION ${MAJOR_VERSION}.${MINOR_VERSION} SOVERSION ${MAJOR_VERSION})
set_target_properties(${STATICLIBTGT} PROPERTIES ARCHIVE_OUTPUT_NAME ${STATICLIB})

if(CMAKE_HOST_APPLE)
  set_target_properties(${SHAREDLIB} PROPERTIES MACOSX_RPATH ON)
endif(CMAKE_HOST_APPLE)

target_include_directories(coreobjs PUBLIC ${CMAKE_SOURCE_DIR}/../include)
target_include_directories(coreobjs PUBLIC ${CMAKE_SOURCE_DIR}/../../nbis/include)

#
# For OS-X, add paths where the ports packages are placed.
#
if(CMAKE_HOST_APPLE)
  target_include_directories(coreobjs PUBLIC /opt/local/include)
  target_link_libraries(${SHAREDLIB} -L/opt/local/lib)
#
# Where to look for Berkeley DB version 4.x
#
  target_include_directories(coreobjs PUBLIC /opt/local/include/db44)
  target_link_libraries(${SHAREDLIB} -L/opt/local/lib/db44)
#
# Apple-provided frameworks
#
  target_link_libraries(${SHAREDLIB} "-framework Foundation" "-framework Security" "-framework PCSC")
  add_definitions("-DDarwin")
else(CMAKE_HOST_APPLE)
  find_package(PCSC)
  if(NOT PCSC_FOUND)
    message(FATAL_ERROR "PCSC not found.")
  endif(NOT PCSC_FOUND)
  include_directories(PUBLIC ${PCSC_INCLUDE_DIR})
  target_link_libraries(${SHAREDLIB} ${PCSC_LIBRARIES})
endif(CMAKE_HOST_APPLE)

#
# OpenJPEG and JPEG2000
#
set(OPENJP2_FIND_REQUIRED TRUE)
find_package(OpenJP2)
if(NOT OPENJP2_FOUND)
  message(FATAL_ERROR "Could not find OpenJP2.")
endif(NOT OPENJP2_FOUND)
include_directories(PUBLIC ${OPENJP2_INCLUDE_DIR})
target_link_libraries(${SHAREDLIB} ${OPENJP2_LIBRARIES})

set(JPEG_FIND_REQUIRED TRUE)
find_package(JPEG)
if(NOT JPEG_FOUND)
  message(FATAL_ERROR "Could not find JPEG.")
endif(NOT JPEG_FOUND)
include_directories(PUBLIC ${JPEG_INCLUDE_DIR})
target_link_libraries(${SHAREDLIB} ${JPEG_LIBRARIES})

#
# FFMPEG
#
if(FFMPEG_FOUND)
  include_directories(PUBLIC ${FFMPEG_INCLUDE_DIR})
  target_link_libraries(${SHAREDLIB} ${FFMPEG_LIBRARIES})
endif(FFMPEG_FOUND)

#
# OpenMPI
#
if(MPI_CXX_FOUND)
  message(STATUS "Adding MPI support to ${SHAREDLIB}.")
  target_link_libraries(${SHAREDLIB} ${MPI_CXX_LINK_FLAGS} ${MPI_CXX_LIBRARIES})
else(MPI_CXX_FOUND)
  message(STATUS "Will build ${SHAREDLIB} without MPI support.")
endif(MPI_CXX_FOUND)

#
# Other libs not specifically searched for above.
#
target_link_libraries(${SHAREDLIB} sqlite3 crypto z db png tiff)

install(TARGETS ${SHAREDLIB} DESTINATION lib)
install(TARGETS ${STATICLIBTGT} DESTINATION lib)
