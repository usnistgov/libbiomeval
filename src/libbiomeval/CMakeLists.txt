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
# To build the Framework:
#     make
# To install built library plus headers:
#     make install
# To create an RPM:
#     make package
#

cmake_minimum_required(VERSION 3.20)

set(BIOMEVAL_C_FLAGS "")
set(BIOMEVAL_CXX_FLAGS "")
set(BIOMEVAL_DEFINITIONS "")

if (BUILD_FOR_WASM)
	list(APPEND BIOMEVAL_DEFINITIONS "WASM")
	if (EMSCRIPTEN)
		list(APPEND BIOMEVAL_C_FLAGS -sUSE_LIBPNG=1 -sUSE_LIBJPEG=1)
		list(APPEND BIOMEVAL_CXX_FLAGS -sUSE_LIBPNG=1 -sUSE_LIBJPEG=1)

		if (WASM_EXCEPTIONS)
			list(APPEND BIOMEVAL_C_FLAGS -sSUPPORT_LONGJMP=wasm)
			list(APPEND BIOMEVAL_CXX_FLAGS -fwasm-exceptions)
			set(LDFLAGS "${LDFLAGS} -fwasm-exceptions")
			set(PNG_NAMES "png-wasm-sjlj")
		endif (WASM_EXCEPTIONS)
	endif (EMSCRIPTEN)

	set(WITH_HWLOC OFF)
endif()

if (BUILD_BIOMEVAL_32BIT)
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)
	list(APPEND BIOMEVAL_C_FLAGS -m32)
	list(APPEND BIOMEVAL_CXX_FLAGS -m32)
endif ()

# Create dirent_windows.h at build time
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../nbis/include/dirent_windows.h")
	if (CMAKE_HOST_UNIX)
		execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/../include/be_dirent_windows.h ${CMAKE_CURRENT_SOURCE_DIR}/../../nbis/include/dirent_windows.h)
	elseif(CMAKE_HOST_WIN32)
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/../include/be_dirent_windows.h" "${CMAKE_CURRENT_SOURCE_DIR}/../../nbis/include/dirent_windows.h")
	endif()
endif ()

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

#
# Define common options
#
set(BIOMEVAL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
file(GLOB BIOMEVAL_HEADERS ${BIOMEVAL_INCLUDE}/*.h)
set(NBIS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../../nbis/include)
#
project(libbiomeval VERSION ${MAJOR_VERSION}.${MINOR_VERSION})

set(CORELIB biomeval)

# NBIS uses "insecure" functions under Windows
if(WIN32)
	# FIXME: Update NBIS to not use these functions
	list(APPEND BIOMEVAL_DEFINITIONS "_CRT_SECURE_NO_WARNINGS" "_CRT_SECURE_NO_WARNINGS_GLOBAL")
endif()

if (BUILD_SHARED_LIBS)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#
# Location of CMake modules local to BIOMEVAL
#
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

set(CORE be_memory_indexedbuffer.cpp be_memory_mutableindexedbuffer.cpp be_text.cpp be_system.cpp be_system_memlog.cpp be_error.cpp be_error_exception.cpp be_time.cpp be_time_timer.cpp be_time_watchdog.cpp be_error_signal_manager.cpp be_framework.cpp be_framework_status.cpp be_framework_api.cpp be_process_statistics.cpp)
set_source_files_properties(be_framework.cpp PROPERTIES
    COMPILE_DEFINITIONS "MAJOR_VERSION=${MAJOR_VERSION};MINOR_VERSION=${MINOR_VERSION}")

set(IO be_io_properties.cpp be_io_propertiesfile.cpp be_io_utility.cpp be_io_logsheet.cpp be_io_filelogsheet.cpp be_io_syslogsheet.cpp be_io_filelogcabinet.cpp be_io_autologger.cpp be_io_compressor.cpp be_io_gzip.cpp)

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
if(WIN32)
    list(REMOVE_ITEM CORE "be_error_signal_manager.cpp" "be_framework_api.cpp" "be_system_memlog.cpp" "be_time_watchdog.cpp" "be_process_statistics.cpp")
    list(REMOVE_ITEM IO "be_io_autologger.cpp" "be_io_syslogsheet.cpp")

    unset(PROCESS)
    unset(MESSAGE_CENTER)

    # Don't include min/max macros from windows.h
    list(APPEND BIOMEVAL_DEFINITIONS NOMINMAX)

    list(APPEND CORE "be_sysdeps.cpp")
endif()

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


set(BIOMEVAL_INCLUDED_FFMPEG OFF)
if (WITH_FFMPEG)
	# Our FindFFMPEG.cmake uses pkg-config, as that is the official method
	# the developers use for specifying dependencies. This is not installed
	# by default on MSVC, but we require vcpkg on Windows, and that installs
	# a copy.
	if (MSVC)
		set(PKG_CONFIG_EXECUTABLE ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/pkgconf/pkgconf.exe --with-path=${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/pkgconfig)
	endif()

	find_package(FFMPEG)

if(FFMPEG_FOUND)
  set(BIOMEVAL_INCLUDED_FFMPEG ON)
  message(STATUS "Adding VIDEO support.")

  add_library(vidobjs OBJECT ${VIDEO})
  target_include_directories(vidobjs PUBLIC ${BIOMEVAL_INCLUDE})
  target_include_directories(vidobjs PRIVATE ${FFMPEG_INCLUDE_DIRS})
else()
  # FIXME: Should we complain if WITH_FFMPEG=YES but FFMPEG is not found?
  message(STATUS "Building without VIDEO support.")
endif()
else()
  message(STATUS "Building without VIDEO support.")
endif()

#
# PCSC (Smartcard) support is an optional library
#
set(BIOMEVAL_INCLUDED_PCSC OFF)
if (WITH_PCSC)
	find_package(PCSC)

if (PCSC_FOUND)
	set(BIOMEVAL_INCLUDED_PCSC ON)
	message(STATUS "Adding PCSC support.")
	add_library(deviceobjs OBJECT ${DEVICE})
	target_include_directories(deviceobjs PUBLIC ${BIOMEVAL_INCLUDE})
else ()
	# FIXME: Should we complain if WITH_PCSC=YES but PCSC is not found?
	message(STATUS "Building without PCSC support.")
endif ()
else()
	message(STATUS "Building without PCSC support.")
endif()

#
# Keep MPI related files separate so we can use a different compiler command.
# MPI files are built as an object-only lib (not linked) so its symbols can
# be added to the BIOMEVAL library later. MPI is optional.
#
set(MPISOURCES ${MPIBASE} ${MPIDISTRIBUTOR} ${MPIRECEIVER})
if(WITH_MPI)
if(NOT MPIEXEC_EXECUTABLE)
	find_program(MPIEXEC_EXECUTABLE mpiexec)
endif()
find_package(MPI)
if(MPI_CXX_FOUND AND NOT WIN32)
  add_library(mpiobjs OBJECT ${MPISOURCES})
  target_compile_definitions(mpiobjs PUBLIC "${MPI_CXX_COMPILE_DEFINITIONS}")
  target_compile_options(mpiobjs PUBLIC "${MPI_CXX_COMPILE_OPTIONS}")
  target_include_directories(
    mpiobjs
    PUBLIC ${MPI_CXX_INCLUDE_DIRS} ${BIOMEVAL_INCLUDE})
else()
  # FIXME: Should we stop if WITH_MPI=ON, but MPI not found?
  message(STATUS "Building without MPI support.")
endif()
else()
  message(STATUS "Building without MPI support.")
endif()

#
# Build the Framework-local version of NBIS
#
file(GLOB_RECURSE NBISSOURCE "${PROJECT_SOURCE_DIR}/../../nbis/lib/*.c")
add_library(nbisobjs OBJECT ${NBISSOURCE})
target_include_directories(nbisobjs PUBLIC ${NBIS_INCLUDE})

#
# Endian check needed for NBIS
#
if(CMAKE_C_BYTE_ORDER STREQUAL "LITTLE_ENDIAN")
  	target_compile_definitions(nbisobjs PRIVATE __NBISLE__)
endif()

#
# Build the core library by building the .cpp
# files into object-only libraries, then combining the object files together
# with the BIOMEVAL and NBIS objects.
#
set(OBJLIBS "")

add_library(coreobjs OBJECT ${PACKAGES})

list(APPEND OBJLIBS $<TARGET_OBJECTS:coreobjs>)
list(APPEND OBJLIBS $<TARGET_OBJECTS:nbisobjs>)
if(MPI_CXX_FOUND AND NOT WIN32)
  list(APPEND OBJLIBS $<TARGET_OBJECTS:mpiobjs>)
endif()
if(FFMPEG_FOUND)
  list(APPEND OBJLIBS $<TARGET_OBJECTS:vidobjs>)
endif()
if(PCSC_FOUND)
  list(APPEND OBJLIBS $<TARGET_OBJECTS:deviceobjs>)
endif()

# "Some native build systems (such as Xcode) may not like targets that have
#  only object files, so consider adding at least one real source file to any
#  target that references $<TARGET_OBJECTS:objlib>".
# https://cmake.org/cmake/help/v3.12/command/add_library.html#object-libraries
if(CMAKE_GENERATOR STREQUAL Xcode)
	set(XCODE_GENERATOR_FIX "be_fix_cmake_xcode_generator.cpp")
	file(WRITE "${XCODE_GENERATOR_FIX}" "")
	list(APPEND OBJLIBS "${XCODE_GENERATOR_FIX}")
endif()

if (BUILD_SHARED_LIBS)
	add_library(${CORELIB} SHARED ${OBJLIBS})
	set_target_properties(${CORELIB} PROPERTIES
            POSITION_INDEPENDENT_CODE ON
            WINDOWS_EXPORT_ALL_SYMBOLS ON)
else()
	add_library(${CORELIB} STATIC ${OBJLIBS})
	set_target_properties(${CORELIB} PROPERTIES
            POSITION_INDEPENDENT_CODE OFF)
endif()

set_target_properties(
    ${CORELIB}
    PROPERTIES VERSION ${PROJECT_VERSION}
    SOVERSION ${MAJOR_VERSION}
    PUBLIC_HEADER "${BIOMEVAL_HEADERS}")

if (FFMPEG_FOUND)
	target_link_libraries(${CORELIB} PRIVATE FFMPEG::FFMPEG)
endif ()

if (WIN32)
	set_target_properties(${CORELIB} PROPERTIES DEBUG_POSTFIX "d")
endif ()

target_include_directories(coreobjs PUBLIC ${BIOMEVAL_INCLUDE} ${NBIS_INCLUDE})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	list(APPEND BIOMEVAL_DEFINITIONS Linux)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	list(APPEND BIOMEVAL_DEFINITIONS Darwin)
endif()

#
# OpenSSL
#
# On macOS, we use the built-in Security framework instead of OpenSSL, but the
# Security framework requires C "block" support, a language extension not found
# in all compilers.
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fblocks" BLOCK_SUPPORT)
set(BIOMEVAL_INCLUDED_OPENSSL OFF)
if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" AND BLOCK_SUPPORT)
	find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
	find_library(SECURITY_FRAMEWORK Security REQUIRED)
	target_link_libraries(${CORELIB} PRIVATE
	    ${FOUNDATION_FRAMEWORK} ${SECURITY_FRAMEWORK})
	list(APPEND BIOMEVAL_DEFINITIONS "UseAppleSecurityFramework")
	message(STATUS "Using Apple Security Framework")
else ()
	find_package(OpenSSL REQUIRED)
	target_include_directories(coreobjs PRIVATE ${OPENSSL_INCLUDE_DIR})
	target_link_libraries(${CORELIB} PRIVATE OpenSSL::Crypto)
	if(NOT OPENSSL_CRYPTO_LIBRARY MATCHES "\\.a$")
	    set(BIOMEVAL_INCLUDED_OPENSSL ON)
	endif()
endif ()

#
# Berkeley DB
#
set(BIOMEVAL_INCLUDED_BERKELEYDB OFF)
if (NOT BUILD_FOR_WASM)
find_package(BERKELEYDB REQUIRED)
target_include_directories(coreobjs PRIVATE ${BERKELEYDB_INCLUDE_DIR})
target_link_libraries(${CORELIB} PRIVATE ${BERKELEYDB_LIBRARIES})
if(NOT BERKELEYDB_LIBRARIES MATCHES "\\.a$")
    set(BIOMEVAL_INCLUDED_BERKELEYDB ON)
endif()
endif()

#
# OpenJPEG and JPEG2000
#
find_package(OpenJPEG REQUIRED)
target_include_directories(coreobjs PRIVATE ${OPENJPEG_INCLUDE_DIRS})
target_link_libraries(${CORELIB} PRIVATE ${OPENJPEG_LIBRARIES})

find_package(JPEG REQUIRED)
target_include_directories(nbisobjs PRIVATE ${JPEG_INCLUDE_DIR})
target_include_directories(coreobjs PRIVATE ${JPEG_INCLUDE_DIR})
target_link_libraries(${CORELIB} PRIVATE JPEG::JPEG)

#
# OpenMPI
#
set(BIOMEVAL_INCLUDED_MPI OFF)
if(MPI_CXX_FOUND AND NOT WIN32)
  message(STATUS "Adding MPI support to ${CORELIB}.")
  if(NOT MPI_CXX_LIBRARIES MATCHES "\\.a$")
      set(BIOMEVAL_INCLUDED_MPI ON)
  endif()
  target_link_libraries(${CORELIB} PRIVATE ${MPI_CXX_LINK_FLAGS} ${MPI_CXX_LIBRARIES})
else()
  message(STATUS "Will build ${CORELIB} without MPI support.")
endif()

#
# PCSC
#
if(PCSC_FOUND)
	if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
		target_include_directories(deviceobjs PUBLIC ${PCSC_INCLUDE_DIR})
	endif()

   target_include_directories(deviceobjs PUBLIC ${BIOMEVAL_INCLUDE})
   target_link_libraries(${CORELIB} PRIVATE PCSC::PCSC)
endif()

#
# The Portable Hardware Locality hwloc
#
set(BIOMEVAL_INCLUDED_HWLOC OFF)
if (WITH_HWLOC)
	find_package(HWLOC REQUIRED)
	target_include_directories(coreobjs PRIVATE ${HWLOC_INCLUDE_DIRS})
	target_link_libraries(${CORELIB} PRIVATE HWLOC::HWLOC)
	list(APPEND BIOMEVAL_DEFINITIONS BIOMEVAL_WITH_HWLOC)
	if(NOT HWLOC_LIBRARY MATCHES "\\.a$")
	    set(BIOMEVAL_INCLUDED_HWLOC ON)
	endif()
else ()
	message(STATUS "Building without HWLOC support.")
endif ()

#
# Other libs not specifically searched for above.
#

# GCC < 9 needs to explicitly link libstdc++fs
target_link_libraries(${CORELIB} PUBLIC
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)

set(BIOMEVAL_INCLUDED_SQLITE3 OFF)
if (NOT BUILD_FOR_WASM)
# This will use our Module
find_package(SQLite3 REQUIRED)
target_link_libraries(${CORELIB} PRIVATE SQLite::SQLite3)
if(NOT SQLite3_LIBRARY MATCHES "\\.a$")
    set(BIOMEVAL_INCLUDED_SQLITE3 ON)
endif()
endif()

find_package(TIFF REQUIRED)
target_include_directories(coreobjs PRIVATE ${TIFF_INCLUDE_DIRS})
target_link_libraries(${CORELIB} PRIVATE TIFF::TIFF)

# Under MacPorts, TIFF requires many things that are not appropriately
# propagated when linking statically.
set(BIOMEVAL_INCLUDED_LERC OFF)
set(BIOMEVAL_INCLUDED_LZMA OFF)
set(BIOMEVAL_INCLUDED_ZSTD OFF)
set(BIOMEVAL_INCLUDED_DEFLATE OFF)
find_file(TIFF_CONFIG_HEADER NAMES tiffconf.h HINTS ${TIFF_INCLUDE_DIR})
if(TIFF_CONFIG_HEADER)
	# Lerc
	file(READ "${TIFF_CONFIG_HEADER}" TIFF_CONFIG)
	string(REGEX MATCH "#define LERC_SUPPORT 1" TIFF_REQUIRES_LERC "${TIFF_CONFIG}")
	if(NOT "${TIFF_REQUIRES_LERC}" STREQUAL "")
		message(VERBOSE "This TIFF library requires Lerc.")
		find_package(Lerc REQUIRED)
		target_link_libraries(${CORELIB} PRIVATE Lerc::Lerc)
		set(BIOMEVAL_INCLUDED_LERC ON)
	endif()

	# LZMA
	find_package(LibLZMA REQUIRED)
	target_link_libraries(${CORELIB} PRIVATE LibLZMA::LibLZMA)
	if(NOT LIBLZMA_LIBRARY MATCHES "\\.a$")
	    set(BIOMEVAL_INCLUDED_LZMA ON)
	endif()

	# Zstd
	find_package(zstd REQUIRED)
	target_link_libraries(${CORELIB} PRIVATE zstd::zstd)
	set(BIOMEVAL_INCLUDED_ZSTD ON)

	# libdeflate
	file(READ "${TIFF_CONFIG_HEADER}" TIFF_CONFIG)
	string(REGEX MATCH "#define LIBDEFLATE_SUPPORT 1" TIFF_REQUIRES_DEFLATE "${TIFF_CONFIG}")
	if(NOT "${TIFF_REQUIRES_DEFLATE}" STREQUAL "")
		message(VERBOSE "This TIFF library requires libdeflate.")
		find_package(deflate REQUIRED)
		target_link_libraries(${CORELIB} PRIVATE deflate::deflate)
		set(BIOMEVAL_INCLUDED_DEFLATE ON)
	endif()
endif()

find_package(PNG REQUIRED)
target_include_directories(coreobjs PRIVATE ${PNG_INCLUDE_DIRS})
target_link_libraries(${CORELIB} PRIVATE PNG::PNG)

# Windows-specific system libraries
if(WIN32)
	# Need Winsock on Windows
	target_link_libraries(${CORELIB} PUBLIC ws2_32)

	# Need crypt32 on static Win32 builds
	if (NOT BUILD_SHARED_LIBS)
		target_link_libraries(${CORELIB} PUBLIC crypt32)
	endif ()
endif()


target_include_directories(${CORELIB} PUBLIC $<INSTALL_INTERFACE:include>)

# Set flags for each target
set(BIOMEVAL_TARGETS "coreobjs;vidobjs;deviceobjs;mpiobjs;nbisobjs;${CORELIB}")
foreach (t ${BIOMEVAL_TARGETS})
	if (TARGET ${t})
		target_compile_options(${t} PRIVATE
		    $<$<COMPILE_LANGUAGE:C>:${BIOMEVAL_C_FLAGS}>
		    $<$<COMPILE_LANGUAGE:CXX>:${BIOMEVAL_CXX_FLAGS}>
		)
		target_compile_definitions(${t} PRIVATE
		    ${BIOMEVAL_DEFINITIONS}
		)

		# Windows needs to differentiate between release/debug builds.
		if (MSVC)
			set_target_properties(${t} PROPERTIES
			    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<BOOL:${BUILD_SHARED_LIBS}>:DLL>")
		endif()
	endif()
endforeach()

#
# Installation.
#
include(GNUInstallDirs)
set(BIOMEVAL_PUBLIC_HEADER_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR})
install(
  TARGETS ${CORELIB}
  EXPORT biomeval-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${BIOMEVAL_PUBLIC_HEADER_INCLUDE_DIR})

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/biomevalConfig.cmake.in
    ${CMAKE_BINARY_DIR}/cmake/biomeval-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/biomeval
    PATH_VARS BIOMEVAL_PUBLIC_HEADER_INCLUDE_DIR
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
endif()
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "NIST Image Group (www.nist.gov/itl/iad/image-group)")
set(CPACK_PACKAGE_VENDOR "NIST (www.nist.gov)")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)
