#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
# To use CMake to create the Biometric Evaluation Framework test programs:
#
# Create a build directory; in this example, it will be under src/test:
#     mkdir build; cd build
#     cmake ..
#     make
#
# If this CMake file is used independently of the library CMake, libbiomeval
# must be built and installed in the common/lib directory.
# If this CMake file is invoked from another CMake file that also includes
# the libbiomeval, then the library will be linked from that build's location.
#

project(biomeval-test)

if (BUILD_BIOMEVAL_32BIT)
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
endif (BUILD_BIOMEVAL_32BIT)

# Sanity check our source directory to make sure that we are not trying to
# generate an in-tree build
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed.
CMake would overwrite the makefiles distributed with the test programs.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
Please delete them.")
endif()

cmake_minimum_required(VERSION 2.8.11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/../libbiomeval/cmake/Modules/")

if(${CMAKE_MAJOR_VERSION} GREATER 2)
    if (MSVC)
        set(CMAKE_CXX_STANDARD 14)
    else (MSVC)
        set(CMAKE_CXX_STANDARD 11)
    endif (MSVC)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
    if(MSVC)
        add_compile_options(/std:c++14)
    else()
        add_compile_options(--std=c++11)
    endif(MSVC)
endif()

#
# The <project>_SOURCE_DIR and <project>_BINARY_DIR are created by the
# CMake config for the framework library, and hence <project> must match
# the project name given on that config file.
#
# TODO: Do not hardcode linkage against static or shared library. Build both?
if(libbiomeval_SOURCE_DIR)
  include_directories(${libbiomeval_SOURCE_DIR}/../include)
  if (CMAKE_GENERATOR STREQUAL Xcode)
    link_libraries(-L${libbiomeval_BINARY_DIR}/$<CONFIGURATION> -lbiomeval)
  elseif(MSVC)
    link_libraries(${libbiomeval_BINARY_DIR}/$<CONFIGURATION>/biomeval$<$<CONFIG:Debug>:d>.lib)
  else(CMAKE_GENERATOR STREQUAL Xcode)
    link_libraries(-L${libbiomeval_BINARY_DIR} -lbiomeval)
  endif(CMAKE_GENERATOR STREQUAL Xcode)
else()
  include_directories(${PROJECT_SOURCE_DIR}/../include)
  if (CMAKE_GENERATOR STREQUAL Xcode)
    link_libraries(-L${CMAKE_BINARY_DIR}/../../$<CONFIGURATION> -lbiomeval)
  elseif(MSVC)
    link_libraries(${CMAKE_BINARY_DIR}/$<CONFIGURATION>/biomeval$<$<CONFIG:Debug>:d>.lib)
  else(CMAKE_GENERATOR STREQUAL Xcode)
    link_libraries(-L${CMAKE_BINARY_DIR}/../../lib/ -lbiomeval)
  endif(CMAKE_GENERATOR STREQUAL Xcode)
endif()

# Windows doesn't have the concept of rpath. DLLs must be in PATH
if(MSVC)
  if (WITH_FFMPEG)
  find_package(FFMPEG)
  if(FFMPEG_FOUND)
    link_libraries(${FFMPEG_LIBRARIES})
  endif(FFMPEG_FOUND)
  endif(WITH_FFMPEG)

  find_package(OPENSSL)
  if(OPENSSL_FOUND)
    link_libraries(${OPENSSL_LIBRARIES})
  endif(OPENSSL_FOUND)

  find_package(BERKELEYDB)
  if(BERKELEYDB_FOUND)
    link_libraries(${BERKELEYDB_LIBRARIES})
  endif(BERKELEYDB_FOUND)

  find_package(JPEG)
  if(JPEG_FOUND)
    link_libraries(${JPEG_LIBRARIES})
  endif(JPEG_FOUND)

  find_package(OPENJP2)
  if(OPENJP2_FOUND)
    link_libraries(${OPENJP2_LIBRARIES})
  endif(OPENJP2_FOUND)

  if(WITH_HWLOC)
  find_package(HWLOC)
  if(HWLOC_FOUND)
    link_libraries(${HWLOC_LIBRARIES})
  endif(HWLOC_FOUND)
  endif(WITH_HWLOC)

  find_package(PNG)
  if(PNG_FOUND)
    link_libraries(${PNG_LIBRARIES})
  endif(PNG_FOUND)

  find_package(TIFF)
  if(TIFF_FOUND)
    link_libraries(${TIFF_LIBRARIES})
  endif(TIFF_FOUND)

  find_package(SQLITE3 REQUIRED)
  if(SQLITE3_FOUND)
    link_libraries(${SQLITE3_LIBRARIES})
  endif(SQLITE3_FOUND)

  # Winsock2
  link_libraries(Ws2_32)
endif(MSVC)

if (BUILD_BIOMEVAL_STATIC)
	set(DEPENDENT_LIB "${STATICLIBTGT}")
else (BUILD_BIOMEVAL_STATIC)
	set(DEPENDENT_LIB "${SHAREDLIB}")
endif (BUILD_BIOMEVAL_STATIC)

function(set_biomeval_test_exe_dependencies TARGET_NAME)
	if (BUILD_BIOMEVAL_STATIC)
		add_dependencies(${TARGET_NAME} ${STATICLIBTGT})
	else (BUILD_BIOMEVAL_STATIC)
		add_dependencies(${TARGET_NAME} ${SHAREDLIB})
	endif (BUILD_BIOMEVAL_STATIC)

	# Windows needs to differentiate between static/dynamic and
	# release/debug builds. TODO above notes that static lib is hardcoded
	# for test executables.
	if (MSVC)
		if (CMAKE_VERSION VERSION_GREATER 3.14.9999)
			set_target_properties(${TARGET_NAME} PROPERTIES
			    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
		else (CMAKE_VERSION VERSION_GREATER 3.14.9999)
			set_target_properties(${TARGET_NAME} PROPERTIES
			    CXX_FLAGS_RELEASE "/MT"
			    CXX_FLAGS_DEBUG "/MTd")
		endif (CMAKE_VERSION VERSION_GREATER 3.14.9999)

		# It also needs to link Crypt32
		target_link_libraries(${TARGET_NAME} Crypt32)
	endif (MSVC)
endfunction(set_biomeval_test_exe_dependencies)

#
# Exhaustively add each target rather than looping over source files. Some
# generators (namely MSVC) don't seem to pick up each target in the loop.
#

# The following executables do not require special treatment
add_executable(test_be_data_interchange_an2k test_be_data_interchange_an2k.cpp)
set_biomeval_test_exe_dependencies(test_be_data_interchange_an2k)
add_executable(test_be_data_interchange_ansi2004 test_be_data_interchange_ansi2004.cpp)
set_biomeval_test_exe_dependencies(test_be_data_interchange_ansi2004)
add_executable(test_be_error test_be_error.cpp)
set_biomeval_test_exe_dependencies(test_be_error)
add_executable(test_be_face_incitsviews test_be_face_incitsviews.cpp)
set_biomeval_test_exe_dependencies(test_be_face_incitsviews)
add_executable(test_be_feature_an2kminutiae test_be_feature_an2kminutiae.cpp)
set_biomeval_test_exe_dependencies(test_be_feature_an2kminutiae)
add_executable(test_be_finger_an2kview test_be_finger_an2kview.cpp)
set_biomeval_test_exe_dependencies(test_be_finger_an2kview)
add_executable(test_be_finger_incitsviews test_be_finger_incitsviews.cpp)
set_biomeval_test_exe_dependencies(test_be_finger_incitsviews)
add_executable(test_be_framework test_be_framework.cpp)
set_biomeval_test_exe_dependencies(test_be_framework)
add_executable(test_be_framework_enumeration test_be_framework_enumeration.cpp)
set_biomeval_test_exe_dependencies(test_be_framework_enumeration)
add_executable(test_be_io_archiverecstore test_be_io_archiverecstore.cpp)
set_biomeval_test_exe_dependencies(test_be_io_archiverecstore)
add_executable(test_be_io_filelogcabinet test_be_io_filelogcabinet.cpp)
set_biomeval_test_exe_dependencies(test_be_io_filelogcabinet)
add_executable(test_be_io_filerecstore test_be_io_filerecstore.cpp)
set_biomeval_test_exe_dependencies(test_be_io_filerecstore)
add_executable(test_be_io_listrecstore test_be_io_listrecstore.cpp)
set_biomeval_test_exe_dependencies(test_be_io_listrecstore)
add_executable(test_be_io_persistentrecordstoreunion test_be_io_persistentrecordstoreunion.cpp)
set_biomeval_test_exe_dependencies(test_be_io_persistentrecordstoreunion)
add_executable(test_be_io_properties test_be_io_properties.cpp)
set_biomeval_test_exe_dependencies(test_be_io_properties)
add_executable(test_be_io_propertiesfile test_be_io_propertiesfile.cpp)
set_biomeval_test_exe_dependencies(test_be_io_propertiesfile)
add_executable(test_be_io_recordstoreunion test_be_io_recordstoreunion.cpp)
set_biomeval_test_exe_dependencies(test_be_io_recordstoreunion)
add_executable(test_be_io_utility test_be_io_utility.cpp)
set_biomeval_test_exe_dependencies(test_be_io_utility)
add_executable(test_be_iris_incitsviews test_be_iris_incitsviews.cpp)
set_biomeval_test_exe_dependencies(test_be_iris_incitsviews)
add_executable(test_be_latent_an2kview test_be_latent_an2kview.cpp)
set_biomeval_test_exe_dependencies(test_be_latent_an2kview)
add_executable(test_be_memory_autoarray test_be_memory_autoarray.cpp)
set_biomeval_test_exe_dependencies(test_be_memory_autoarray)
add_executable(test_be_memory_indexedbuffer test_be_memory_indexedbuffer.cpp)
set_biomeval_test_exe_dependencies(test_be_memory_indexedbuffer)
add_executable(test_be_memory_orderedmap test_be_memory_orderedmap.cpp)
set_biomeval_test_exe_dependencies(test_be_memory_orderedmap)
add_executable(test_be_palm_an2kview test_be_palm_an2kview.cpp)
set_biomeval_test_exe_dependencies(test_be_palm_an2kview)
add_executable(test_be_system test_be_system.cpp)
set_biomeval_test_exe_dependencies(test_be_system)
add_executable(test_be_text test_be_text.cpp)
set_biomeval_test_exe_dependencies(test_be_text)
add_executable(test_be_time test_be_time.cpp)
set_biomeval_test_exe_dependencies(test_be_time)
add_executable(test_be_time_timer test_be_time_timer.cpp)
set_biomeval_test_exe_dependencies(test_be_time_timer)

# Individual RecordStore test executables (requires compiler definition)
add_executable(test_be_io_filerecordstore test_be_io_recordstore.cpp)
set_biomeval_test_exe_dependencies(test_be_io_filerecordstore)
target_compile_definitions(test_be_io_filerecordstore PUBLIC FILERECORDSTORETEST)
add_executable(test_be_io_dbrecordstore test_be_io_recordstore.cpp)
set_biomeval_test_exe_dependencies(test_be_io_dbrecordstore)
target_compile_definitions(test_be_io_dbrecordstore PUBLIC DBRECORDSTORETEST)
add_executable(test_be_io_archiverecordstore test_be_io_recordstore.cpp)
set_biomeval_test_exe_dependencies(test_be_io_archiverecordstore)
target_compile_definitions(test_be_io_archiverecordstore PUBLIC ARCHIVERECORDSTORETEST)
add_executable(test_be_io_sqliterecordstore test_be_io_recordstore.cpp)
set_biomeval_test_exe_dependencies(test_be_io_sqliterecordstore)
target_compile_definitions(test_be_io_sqliterecordstore PUBLIC SQLITERECORDSTORETEST)
add_executable(test_be_io_compressedrecordstore test_be_io_recordstore.cpp)
set_biomeval_test_exe_dependencies(test_be_io_compressedrecordstore)
target_compile_definitions(test_be_io_compressedrecordstore PUBLIC COMPRESSEDRECORDSTORETEST)

# Individual RecordStore stress-test executables (requires compiler definition)
add_executable(test_be_io_filerecordstore-stress test_be_io_recordstore-stress.cpp)
set_biomeval_test_exe_dependencies(test_be_io_filerecordstore-stress)
target_compile_definitions(test_be_io_filerecordstore-stress PUBLIC FILERECORDSTORETEST)
add_executable(test_be_io_dbrecordstore-stress test_be_io_recordstore-stress.cpp)
set_biomeval_test_exe_dependencies(test_be_io_dbrecordstore-stress)
target_compile_definitions(test_be_io_dbrecordstore-stress PUBLIC DBRECORDSTORETEST)
add_executable(test_be_io_archiverecordstore-stress test_be_io_recordstore-stress.cpp)
set_biomeval_test_exe_dependencies(test_be_io_archiverecordstore-stress)
target_compile_definitions(test_be_io_archiverecordstore-stress PUBLIC ARCHIVERECORDSTORETEST)
add_executable(test_be_io_sqliterecordstore-stress test_be_io_recordstore-stress.cpp)
set_biomeval_test_exe_dependencies(test_be_io_sqliterecordstore-stress)
target_compile_definitions(test_be_io_sqliterecordstore-stress PUBLIC SQLITERECORDSTORETEST)

# Individual Image format test executables (requires compiler definition)
add_executable(test_be_image_raw test_be_image_image.cpp)
set_biomeval_test_exe_dependencies(test_be_image_raw)
target_compile_definitions(test_be_image_raw PUBLIC RAWTEST)
add_executable(test_be_image_jpeg test_be_image_image.cpp)
set_biomeval_test_exe_dependencies(test_be_image_jpeg)
target_compile_definitions(test_be_image_jpeg PUBLIC JPEGBTEST)
add_executable(test_be_image_jpegl test_be_image_image.cpp)
set_biomeval_test_exe_dependencies(test_be_image_jpegl)
target_compile_definitions(test_be_image_jpegl PUBLIC JPEGLTEST)
add_executable(test_be_image_jpeg2000 test_be_image_image.cpp)
set_biomeval_test_exe_dependencies(test_be_image_jpeg2000)
target_compile_definitions(test_be_image_jpeg2000 PUBLIC JPEG2000TEST)
add_executable(test_be_image_jpeg2000l test_be_image_image.cpp)
set_biomeval_test_exe_dependencies(test_be_image_jpeg2000l)
target_compile_definitions(test_be_image_jpeg2000l PUBLIC JPEG2000LTEST)
add_executable(test_be_image_netpbm test_be_image_image.cpp)
set_biomeval_test_exe_dependencies(test_be_image_netpbm)
target_compile_definitions(test_be_image_netpbm PUBLIC NETPBMTEST)
add_executable(test_be_image_png test_be_image_image.cpp)
set_biomeval_test_exe_dependencies(test_be_image_png)
target_compile_definitions(test_be_image_png PUBLIC PNGTEST)
add_executable(test_be_image_wsq test_be_image_image.cpp)
set_biomeval_test_exe_dependencies(test_be_image_wsq)
target_compile_definitions(test_be_image_wsq PUBLIC WSQTEST)
add_executable(test_be_image_bmp test_be_image_image.cpp)
set_biomeval_test_exe_dependencies(test_be_image_bmp)
target_compile_definitions(test_be_image_bmp PUBLIC BMPTEST)
add_executable(test_be_image_tiff test_be_image_image.cpp)
set_biomeval_test_exe_dependencies(test_be_image_tiff)
target_compile_definitions(test_be_image_tiff PUBLIC TIFFTEST)
add_executable(test_be_image_factory test_be_image_image.cpp)
set_biomeval_test_exe_dependencies(test_be_image_factory)
target_compile_definitions(test_be_image_factory PUBLIC FACTORYTEST)

# Individual process manager executables (requires compiler definition)
if (NOT MSVC)
	add_executable(test_be_process_forkmanager test_be_process_manager.cpp)
	set_biomeval_test_exe_dependencies(test_be_process_forkmanager)
	target_compile_definitions(test_be_process_forkmanager PUBLIC FORKTEST)
	add_executable(test_be_process_posixthreadmanager test_be_process_manager.cpp)
	set_biomeval_test_exe_dependencies(test_be_process_posixthreadmanager)
	target_compile_definitions(test_be_process_posixthreadmanager PUBLIC POSIXTHREADTEST)
endif (NOT MSVC)

# MPI test executables require that MPI support was compiled in
if (TARGET mpiobjs)
	# We also have not tested MPI on Windows
	if (NOT MSVC)
		add_executable(test_be_csv_mpi test_be_csv_mpi.cpp)
		set_biomeval_test_exe_dependencies(test_be_csv_mpi)
		add_executable(test_be_rs_mpi test_be_rs_mpi.cpp)
		set_biomeval_test_exe_dependencies(test_be_rs_mpi)
	endif (NOT MSVC)
endif (TARGET mpiobjs)

# ISO/IEC 7816 test executables require that device support was compiled in
if (TARGET deviceobjs)
	add_executable(test_be_device_smartcard test_be_device_smartcard.cpp)
	set_biomeval_test_exe_dependencies(test_be_device_smartcard)
	add_executable(test_be_device_tlv test_be_device_tlv.cpp)
	set_biomeval_test_exe_dependencies(test_be_device_tlv)
endif (TARGET deviceobjs)

# Process and video test programs require a threading library, which we
# haven't tested under Windows
if (NOT MSVC)
	add_executable(test_be_process_semaphore test_be_process_semaphore.cpp)
	set_biomeval_test_exe_dependencies(test_be_process_semaphore)
	add_executable(test_be_process_statistics test_be_process_statistics.cpp)
	set_biomeval_test_exe_dependencies(test_be_process_statistics)

	# Video test executables require that video support was compiled in
	if (TARGET vidobjs)
		add_executable(test_be_video test_be_video.cpp)
		set_biomeval_test_exe_dependencies(test_be_video)
	endif (TARGET vidobjs)

	find_package(Threads REQUIRED)
	if (${CMAKE_VERSION} VERSION_GREATER 3.0.9999)
		target_link_libraries(test_be_process_semaphore PRIVATE Threads::Threads)
		target_link_libraries(test_be_process_statistics PRIVATE Threads::Threads)
		if (TARGET test_be_video)
			target_link_libraries(test_be_video PRIVATE Threads::Threads)
		endif (TARGET test_be_video)
	else (${CMAKE_VERSION} VERSION_GREATER 3.0.9999)
		if (CMAKE_THREAD_LIBS_INIT)
			target_link_libraries(test_be_process_semaphore "${CMAKE_THREAD_LIBS_INIT}")
			target_link_libraries(test_be_process_statistics "${CMAKE_THREAD_LIBS_INIT}")
			if (TARGET test_be_video)
				target_link_libraries(test_be_video "${CMAKE_THREAD_LIBS_INIT}")
			endif (TARGET test_be_video)
		endif (CMAKE_THREAD_LIBS_INIT)
	endif (${CMAKE_VERSION} VERSION_GREATER 3.0.9999)
endif (NOT MSVC)

# Some test executables are not yet supported on Windows because we have not
# compiled in the relevant classes from libbiomeval. We can't
# EXCLUDE_FROM_ALL, because CMake only excludes from the ALL_BUILD target on
# newer versions of Visual Studio and not the sln target, which is the
# default build target.
if (NOT MSVC)
	add_executable(test_be_error_signal_manager test_be_error_signal_manager.cpp)
	set_biomeval_test_exe_dependencies(test_be_error_signal_manager)
	add_executable(test_be_framework_api test_be_framework_api.cpp)
	set_biomeval_test_exe_dependencies(test_be_framework_api)
	add_executable(test_be_io_syslogsheet test_be_io_syslogsheet.cpp)
	set_biomeval_test_exe_dependencies(test_be_io_syslogsheet)
	add_executable(test_be_time_watchdog test_be_time_watchdog.cpp)
	set_biomeval_test_exe_dependencies(test_be_time_watchdog)
endif (NOT MSVC)

#
# Link the test data into the build directory.
#
if (NOT EXISTS test_data)
	if (UNIX)
		execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${PROJECT_SOURCE_DIR}/test_data test_data)
	elseif(WIN32)
		execute_process(COMMAND mklink /J test_data ${PROJECT_SOURCE_DIR}/test_data)
	endif()
endif (NOT EXISTS test_data)
