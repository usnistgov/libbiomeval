#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#

set(MAJOR_VERSION 10)
set(MINOR_VERSION 0)
set(CORELIB biomeval)
set(SHAREDLIB ${CORELIB}.${MAJOR_VERSION}.${MINOR_VERSION})
project(BIOMEVAL)

#
# Location of CMake modules local to BIOMEVAL
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Sanity check our source directory to make sure that we are not trying to
# generate an in-tree build
if( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE )
  message(FATAL_ERROR "In-source builds are not allowed.
CMake would overwrite the makefiles distributed with the Framework.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
Please delete them.")
endif()

cmake_minimum_required(VERSION 3.7)

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
# All the packages for the core library, except MPI which is linked in later
#
set(PACKAGES ${CORE} ${IO} ${RECORDSTORE} ${IMAGE} ${FEATURE} ${VIEW} ${DATA} ${FINGER} ${PALM} ${PLANTAR} ${IRIS} ${FACE} ${PROCESS} ${VIDEO} ${DEVICE} ${MESSAGE_CENTER})

#
# Keep MPI related files separate so we can use a different compiler command.
# MPI files are built as an object-only lib (not linked) so its symbols can
# be added to the BIOMEVAL library later.
#
set(MPISOURCES ${MPIBASE} ${MPIDISTRIBUTOR} ${MPIRECEIVER})
find_package(MPI)
if(NOT MPI_CXX_FOUND)
  message(FATAL_ERROR "MPI compiler for C++ not found!")
else(NOT MPI_CXX_FOUND)
  add_library(mpiobjs OBJECT ${MPISOURCES})
  set_target_properties(mpiobjs PROPERTIES CXX_COMPILER_LAUNCHER "${MPI_CXX_COMPILER}")
  set_target_properties(mpiobjs PROPERTIES COMPILE_FLAGS "${MPI_CXX_COMPILE_FLAGS}")
  target_include_directories(mpiobjs PUBLIC ${CMAKE_SOURCE_DIR}/../include)
  target_compile_options(mpiobjs PUBLIC --std=c++11 -fPIC)
endif(NOT MPI_CXX_FOUND)

#
# Build the core library, shared and static versions, by building the .cpp
# files into object-only libraries, then combing the object files together.
#
add_library(coreobjs OBJECT ${PACKAGES})
if(MPI_CXX_FOUND)
  add_library(${SHAREDLIB} SHARED $<TARGET_OBJECTS:coreobjs> $<TARGET_OBJECTS:mpiobjs>)
  add_library(${CORELIB} STATIC $<TARGET_OBJECTS:coreobjs> $<TARGET_OBJECTS:mpiobjs>)
else(MPI_CXX_FOUND)
  add_library(${SHAREDLIB} SHARED $<TARGET_OBJECTS:coreobjs>)
  add_library(${CORELIB} STATIC $<TARGET_OBJECTS:coreobjs>)
endif(MPI_CXX_FOUND)

#
# Define common options, including that for Berkeley DB version < 2
#
target_compile_options(coreobjs PUBLIC --std=c++11 -DMAJOR_VERSION -DMINOR_VERSION -DDB1X -fPIC)
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
  target_compile_definitions(coreobjs PUBLIC Darwin)
else(CMAKE_HOST_APPLE)
  find_package(PCSC)
  if(PCSC_FOUND)
    include_directories(PUBLIC ${PCSC_INCLUDE_DIR})
    target_link_libraries(${SHAREDLIB} ${PCSC_LIBRARIES})
  endif(PCSC_FOUND)
endif(CMAKE_HOST_APPLE)

#
# OpenJPEG and JPEG2000
#
set(OPENJP2_FIND_REQUIRED TRUE)
find_package(OpenJP2)
if(OPENJP2_FOUND)
  include_directories(PUBLIC ${OPENJP2_INCLUDE_DIR})
  target_link_libraries(${SHAREDLIB} ${OPENJP2_LIBRARIES})
endif (OPENJP2_FOUND)

set(JPEG_FIND_REQUIRED TRUE)
find_package(JPEG)
if(JPEG_FOUND)
  include_directories(PUBLIC ${JPEG_INCLUDE_DIR})
  target_link_libraries(${SHAREDLIB} ${JPEG_LIBRARIES})
endif (JPEG_FOUND)

#
# FFMPEG
#
set(FFMPEG_FIND_REQUIRED TRUE)
find_package(FFMPEG)
if(FFMPEG_FOUND)
  include_directories(PUBLIC ${FFMPEG_INCLUDE_DIR})
  target_link_libraries(${SHAREDLIB} ${FFMPEG_LIBRARIES})
endif (FFMPEG_FOUND)

#
# The NBIS library built for BIOMEVAL
#
target_link_libraries(${SHAREDLIB} -L${CMAKE_SOURCE_DIR}/../../nbis/lib biomeval_nbis)

#
# OpenMPI
#
if(MPI_CXX_FOUND)
  message(STATUS "Adding MPI symbols to ${SHAREDLIB}")
  target_link_libraries(${SHAREDLIB} ${MPI_CXX_LINK_FLAGS} ${MPI_CXX_LIBRARIES})
endif(MPI_CXX_FOUND)

#
# Other libs not specifically searched for above.
#
target_link_libraries(${SHAREDLIB} sqlite3 crypto z db png tiff)

install(TARGETS ${SHAREDLIB} DESTINATION lib)
