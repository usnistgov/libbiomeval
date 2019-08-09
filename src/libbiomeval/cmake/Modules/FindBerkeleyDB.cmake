# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
#
# Created by NIST for the Biometric Evaluation Framework.
#
#.rst:
# FindBerkeleyDB.cmake
# --------
#
# Find Berkeley Database.
#
# Find the PC/SC smartcard library and headers.
#
# ::
#
#   BDB_INCLUDE_DIR, where to find pcsclite.h, etc.
#   BDB_LIBRARIES, the libraries needed to use BDB.
#   BDB_FOUND, If false, do not try to use BDB.
#
# also defined, but not for general use are
#
# ::
#
#   BDB_LIBRARY, where to find the PC/SC library.

find_path(BDB_INCLUDE_DIR db_cxx.h
  /usr/include/
  /usr/local/include/
)

set(BDB_NAMES db_cxx libdb_cxx)
find_library(BDB_LIBRARY NAMES ${BDB_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set BDB_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BDB DEFAULT_MSG BDB_LIBRARY BDB_INCLUDE_DIR)

if(BDB_FOUND)
  set(BDB_LIBRARIES ${BDB_LIBRARY})
endif()

mark_as_advanced(BDB_LIBRARY BDB_INCLUDE_DIR )
