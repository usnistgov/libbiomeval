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
# must be built and installed in the common/lib directoy. 
# If this CMake file is invoked from another CMake file that also includes
# the libbiomeval, then the library will be linked from that build's location.
#

project(biomeval-test)

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

file(GLOB sources "${PROJECT_SOURCE_DIR}/test_*.cpp")

#
# Test programs are always built with debug turned on
#
if(${CMAKE_MAJOR_VERSION} GREATER 2)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    if(MSVC)
        add_compile_options(/DEBUG /Zi /MDd)
    endif(MSVC)
else()
    if(MSVC)
        add_compile_options(/std:c++14 /DEBUG /Zi /MDd)
    else()
        add_compile_options(--std=c++11 -g)
    endif(MSVC)
endif()

#
# The <project>_SOURCE_DIR and <project>_BINARY_DIR are created by the
# CMake config for the framework library, and hence <project> must match
# the project name given on that config file.
#
if(libbiomeval_SOURCE_DIR)
  include_directories(${libbiomeval_SOURCE_DIR}/../include)
  if(MSVC)
    link_libraries(${libbiomeval_BINARY_DIR}/$<CONFIGURATION>/biomeval.lib)
  else(MSVC)
    link_libraries(-L${libbiomeval_BINARY_DIR} -lbiomeval)
  endif(MSVC)
else()
  include_directories(${PROJECT_SOURCE_DIR}/../include)
  if(MSVC)
    link_libraries(${CMAKE_BINARY_DIR}/$<CONFIGURATION>/biomeval.lib)
  else(MSVC)
    link_libraries(-L${CMAKE_BINARY_DIR}/../../lib/ -lbiomeval)
  endif(MSVC)
endif()

# Windows doesn't have the concept of rpath. DLLs must be in PATH
if(MSVC)
  find_package(FFMPEG)
  if(FFMPEG_FOUND)
    link_libraries(${FFMPEG_LIBRARIES})
  endif(FFMPEG_FOUND)

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

  find_package(HWLOC)
  if(HWLOC_FOUND)
    link_libraries(${HWLOC_LIBRARIES})
  endif(HWLOC_FOUND)

  find_package(PNG)
  if(PNG_FOUND)
    link_libraries(${PNG_LIBRARIES})
  endif(PNG_FOUND)

  find_package(TIFF)
  if(TIFF_FOUND)
    link_libraries(${TIFF_LIBRARIES})
  endif(TIFF_FOUND)

  if(${CMAKE_VERSION} VERSION_LESS "3.14")
	find_package(SQLITE3 REQUIRED)
	link_libraries(${SQLITE3_LIBRARIES})
  else()
	find_package(SQLite3 REQUIRED)
	if(MSVC)
		link_libraries(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/sqlite3.lib)
	else(MSVC)
		link_libraries(${SQLite3_LIBRARIES})
	endif(MSVC)
  endif()

  # Winsock2
  link_libraries(Ws2_32)
endif(MSVC)

foreach(src IN LISTS sources)
  set(build_it 1)
  get_filename_component(exec ${src} NAME_WE)

  #
  # Some test programs have special compiler options.
  #
  if(${exec} STREQUAL test_be_io_recordstore)
    add_executable(test_be_io_filerecordstore ${src})
    target_compile_definitions(test_be_io_filerecordstore PUBLIC FILERECORDSTORETEST)
    add_executable(test_be_io_dbrecordstore ${src})
    target_compile_definitions(test_be_io_dbrecordstore PUBLIC DBRECORDSTORETEST)
    add_executable(test_be_io_archiverecordstore ${src})
    target_compile_definitions(test_be_io_archiverecordstore PUBLIC ARCHIVERECORDSTORETEST)
    add_executable(test_be_io_sqliterecordstore ${src})
    target_compile_definitions(test_be_io_sqliterecordstore PUBLIC SQLITERECORDSTORETEST)
    add_executable(test_be_io_compressedrecordstore ${src})
    target_compile_definitions(test_be_io_compressedrecordstore PUBLIC COMPRESSEDRECORDSTORETEST)
    #
    # Don't build the executable based on the file name
    #
    unset(build_it)
  endif()

  if(${exec} STREQUAL test_be_io_recordstore-stress)
    add_executable(test_be_io_filerecordstore-stress ${src})
    target_compile_definitions(test_be_io_filerecordstore-stress PUBLIC FILERECORDSTORETEST)
    add_executable(test_be_io_dbrecordstore-stress ${src})
    target_compile_definitions(test_be_io_dbrecordstore-stress PUBLIC DBRECORDSTORETEST)
    add_executable(test_be_io_archiverecordstore-stress ${src})
    target_compile_definitions(test_be_io_archiverecordstore-stress PUBLIC ARCHIVERECORDSTORETEST)
    add_executable(test_be_io_sqliterecordstore-stress ${src})
    target_compile_definitions(test_be_io_sqliterecordstore-stress PUBLIC SQLITERECORDSTORETEST)
    #
    # Don't build the executable based on the file name
    #
    unset(build_it)
  endif()

  if(${exec} STREQUAL test_be_image_image)
    add_executable(test_be_image_raw ${src})
    target_compile_definitions(test_be_image_raw PUBLIC RAWTEST)
    add_executable(test_be_image_jpeg ${src})
    target_compile_definitions(test_be_image_jpeg PUBLIC JPEGBTEST)
    add_executable(test_be_image_jpegl ${src})
    target_compile_definitions(test_be_image_jpegl PUBLIC JPEGLTEST)
    add_executable(test_be_image_jpeg2000 ${src})
    target_compile_definitions(test_be_image_jpeg2000 PUBLIC JPEG2000TEST)
    add_executable(test_be_image_jpeg2000l ${src})
    target_compile_definitions(test_be_image_jpeg2000l PUBLIC JPEG2000LTEST)
    add_executable(test_be_image_netpbm ${src})
    target_compile_definitions(test_be_image_netpbm PUBLIC NETPBMTEST)
    add_executable(test_be_image_png ${src})
    target_compile_definitions(test_be_image_png PUBLIC PNGTEST)
    add_executable(test_be_image_wsq ${src})
    target_compile_definitions(test_be_image_wsq PUBLIC WSQTEST)
    add_executable(test_be_image_bmp ${src})
    target_compile_definitions(test_be_image_bmp PUBLIC BMPTEST)
    add_executable(test_be_image_tiff ${src})
    target_compile_definitions(test_be_image_tiff PUBLIC TIFFTEST)
    add_executable(test_be_image_factory ${src})
    target_compile_definitions(test_be_image_factory PUBLIC FACTORYTEST)
    #
    # Don't build the executable based on the file name
    #
    unset(build_it)
  endif()

  if(${exec} STREQUAL test_be_video)
    if(NOT TARGET vidobjs)
      unset(build_it)
    endif()
  endif()

  if(${exec} STREQUAL test_be_rs_mpi OR ${exec} STREQUAL test_be_csv_mpi)
    if(NOT TARGET mpiobjs)
      unset(build_it)
    endif()
  endif()

  if(${exec} STREQUAL test_be_device_smartcard OR ${exec} STREQUAL test_be_device_tlv)
    if(NOT TARGET deviceobjs)
      unset(build_it)
    endif()
  endif()

  if(${exec} STREQUAL test_be_process_manager AND NOT MSVC)
    add_executable(test_be_process_forkmanager ${src})
    target_compile_definitions(test_be_process_forkmanager PUBLIC FORKTEST)
    add_executable(test_be_process_posixthreadmanager ${src})
    target_compile_definitions(test_be_process_posixthreadmanager PUBLIC POSIXTHREADTEST)
  endif()

  if(MSVC)
    STRING(FIND "${exec}" "_process" FOUND_STR)
    if(FOUND_STR)
        unset(build_it)
    endif(FOUND_STR)
    unset(FOUND_STR)

    STRING(FIND "${exec}" "_mpi" FOUND_STR)
    if(FOUND_STR)
        unset(build_it)
    endif(FOUND_STR)
    unset(FOUND_STR)

    STRING(FIND "${exec}" "_mc" FOUND_STR)
    if(FOUND_STR)
        unset(build_it)
    endif(FOUND_STR)
    unset(FOUND_STR)

    STRING(FIND "${exec}" "_message" FOUND_STR)
    if(FOUND_STR)
        unset(build_it)
    endif(FOUND_STR)
    unset(FOUND_STR)

    if("${exec}" STREQUAL "test_be_framework_api")
        unset(build_it)
    endif("${exec}" STREQUAL "test_be_framework_api")

    if("${exec}" STREQUAL "test_be_error_signal_processor")
        unset(build_it)
    endif("${exec}" STREQUAL "test_be_error_signal_processor")

    if("${exec}" STREQUAL "test_be_io_syslogsheet")
        unset(build_it)
    endif("${exec}" STREQUAL "test_be_io_syslogsheet")
  endif(MSVC)

  #
  # The default build, executable named after source file name.
  #
  if(build_it)
    add_executable(${exec} ${src})
  endif()

  #
  # Options needed for certain test programs
  #
  if(${exec} STREQUAL test_be_process_statistics AND NOT MSVC)
    target_link_libraries(${exec} pthread)
  endif()
  if(${exec} STREQUAL test_be_process_semaphore AND NOT MSVC)
    target_link_libraries(${exec} pthread)
  endif()

endforeach(src)

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
