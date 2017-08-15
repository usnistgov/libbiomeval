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

find_path(PCSC_INCLUDE_DIR pcsclite.h
  /usr/include/PCSC
  /usr/local/include/PCSC
  /usr/include/
  /usr/local/include/
)

set(PCSC_NAMES pcsclite libpcsclite)
find_library(PCSC_LIBRARY NAMES ${PCSC_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set PCSC_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PCSC DEFAULT_MSG PCSC_LIBRARY PCSC_INCLUDE_DIR)

if(PCSC_FOUND)
  set(PCSC_LIBRARIES ${PCSC_LIBRARY})
endif()

mark_as_advanced(PCSC_LIBRARY PCSC_INCLUDE_DIR )
