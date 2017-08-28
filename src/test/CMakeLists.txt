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

file(GLOB sources "${PROJECT_SOURCE_DIR}/test_*.cpp")

#
# Test programs are always built with debug turned on
#
add_compile_options(--std=c++11 -g)

#
# The <project>_SOURCE_DIR and <project>_BINARY_DIR are created by the
# CMake config for the framework library, and hence <project> must match
# the project name given on that config file.
#
if(libbiomeval_SOURCE_DIR)
  include_directories(${libbiomeval_SOURCE_DIR}/../include)
  link_libraries(-L${libbiomeval_BINARY_DIR} -lbiomeval)
else()
  include_directories(${PROJECT_SOURCE_DIR}/../include)
  link_libraries(-L${CMAKE_SOURCE_DIR}/../../lib/ -lbiomeval)
endif()

foreach(src IN LISTS sources)
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
    continue()
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
    continue()
  endif()
  if(${exec} STREQUAL test_be_image_image)
    continue()
  endif()
  if(${exec} STREQUAL test_be_process_manager)
    add_executable(test_be_process_forkmanager ${src})
    target_compile_definitions(test_be_process_forkmanager PUBLIC FORKTEST)
    add_executable(test_be_process_posixthreadmanager ${src})
    target_compile_definitions(test_be_process_posixthreadmanager PUBLIC POSIXTHREADTEST)
    continue()
  endif()

  #
  # The default build, executable named after source file name.
  #
  add_executable(${exec} ${src})

  #
  # Options needed for certain test programs
  #
  if(${exec} STREQUAL test_be_process_statistics)
    target_link_libraries(${exec} pthread)
  endif()
  if(${exec} STREQUAL test_be_process_semaphore)
    target_link_libraries(${exec} pthread)
  endif()

endforeach(src)

#
# Link the test data into the build directory.
#
execute_process(COMMAND ln -s ${PROJECT_SOURCE_DIR}/test_data test_data)
