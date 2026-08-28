# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
#
# Created by NIST for the Biometric Evaluation Framework.
#
#.rst:
# FindPCSC
# --------
#
# Find PC/SC.
#
# Find the PC/SC smartcard library and headers.
#
# ::
#
#   PCSC_INCLUDE_DIR, where to find pcsclite.h, etc.
#   PCSC_LIBRARIES, the libraries needed to use PCSC.
#   PCSC_FOUND, If false, do not try to use PCSC.
#
# also defined, but not for general use are
#
# ::
#
#   PCSC_LIBRARY, where to find the PC/SC library.
#
# On Darwin systems, PCSC_INCLUDE_DIR is not defined, because PCSC is included
# as a Framework.
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# PCSC::PCSC
#

include(FindPackageHandleStandardArgs)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	find_library(PCSC_LIBRARY NAMES PCSC)
	find_package_handle_standard_args(PCSC DEFAULT_MSG PCSC_LIBRARY)

else()
	find_path(PCSC_INCLUDE_DIR pcsclite.h
	    /usr/include/PCSC
	    /usr/local/include/PCSC
	)
	find_library(PCSC_LIBRARY NAMES pcsclite libpcsclite)

	find_package_handle_standard_args(PCSC DEFAULT_MSG PCSC_LIBRARY PCSC_INCLUDE_DIR)
endif()

if(PCSC_FOUND)
	if(NOT TARGET PCSC::PCSC)
		add_library(PCSC::PCSC UNKNOWN IMPORTED)
		set_target_properties(PCSC::PCSC PROPERTIES
		    IMPORTED_LOCATION "${PCSC_LIBRARY}")

		if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
			set_target_properties(PCSC::PCSC PROPERTIES
			    INTERFACE_INCLUDE_DIRECTORIES "${PCSC_INCLUDE_DIR}")
		endif()
	endif()

	set(PCSC_LIBRARIES ${PCSC_LIBRARY})
endif()

mark_as_advanced(PCSC_LIBRARY PCSC_INCLUDE_DIR)
