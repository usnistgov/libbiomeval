# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
#
# Created by NIST for the Biometric Evaluation Framework.
#
#.rst:
# FindBerkeleyDB
# --------
#
# Find BerkeleyDB 
#
# Find the Berkeley database C++ includes and library This module defines
#
# ::
#
#   BERKELEYDB_INCLUDE_DIR, where to find db_cxx.h, etc.
#   BERKELEYDB_LIBRARIES, the libraries needed to use BDB.
#   BERKELEYDB_FOUND, If false, do not try to use BDB.
#
# also defined, but not for general use are
#
# ::
#
#   BERKELEYDB_LIBRARY, where to find the library.

if(UNIX)
	find_path(BERKELEYDB_INCLUDE_DIR db_cxx.h
	  /usr/include/
	  /opt/local/include/db48/
	  /opt/local/include/db62/
	)
elseif(WIN32)
	if(_VCPKG_INSTALLED_DIR AND VCPKG_TARGET_TRIPLET)
		find_path(BERKELEYDB_INCLUDE_DIR db_cxx.h
			${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
		)
	endif(_VCPKG_INSTALLED_DIR AND VCPKG_TARGET_TRIPLET)
endif()

#
# IMPORTANT: Make sure the library search paths are in the same version order
# as the above include search paths.
#
if(UNIX)
	find_library(BERKELEYDB_LIBRARY NAMES db_cxx PATHS
	  /opt/local/lib/db48/
	  /opt/local/lib/db62/
	)
elseif(WIN32)
	if(_VCPKG_INSTALLED_DIR AND VCPKG_TARGET_TRIPLET)
		find_library(BERKELEYDB_DEBUG_LIBRARY NAMES libdb48.lib PATHS
			${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib
			NO_DEFAULT_PATH
		)
		find_library(BERKELEYDB_RELEASE_LIBRARY NAMES libdb48.lib PATHS
			${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib
			NO_DEFAULT_PATH
		)
		if (BERKELEYDB_RELEASE_LIBRARY AND BERKELEYDB_DEBUG_LIBRARY)
			set(BERKELEYDB_LIBRARY optimized ${BERKELEYDB_RELEASE_LIBRARY} debug ${BERKELEYDB_DEBUG_LIBRARY})
		elseif(BERKELEYDB_RELEASE_LIBRARY)
			set(BERKELEYDB_LIBRARY optimized ${BERKELEYDB_RELEASE_LIBRARY})
		elseif(BERKELEYDB_DEBUG_LIBRARY)
			set(BERKELEYDB_LIBRARY debug ${BERKELEYDB_DEBUG_LIBRARY})
		endif (BERKELEYDB_RELEASE_LIBRARY AND BERKELEYDB_DEBUG_LIBRARY)
	endif(_VCPKG_INSTALLED_DIR AND VCPKG_TARGET_TRIPLET)
endif()

# handle the QUIETLY and REQUIRED arguments and set BERKELEYDB_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BERKELEYDB DEFAULT_MSG BERKELEYDB_LIBRARY BERKELEYDB_INCLUDE_DIR)

if(BERKELEYDB_FOUND)
  set(BERKELEYDB_LIBRARIES ${BERKELEYDB_LIBRARY})
endif()

mark_as_advanced(BERKELEYDB_LIBRARY BERKELEYDB_INCLUDE_DIR)
