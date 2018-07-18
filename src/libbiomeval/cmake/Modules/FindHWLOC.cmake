# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
#
# Created by NIST for the Biometric Evaluation Framework.
#
#.rst:
# FindHWLOC
# --------
#
# Find HWLOC, the Portable Hardware Locality library.
#
# Find the HWLOC library and headers.
#
# ::
#
#   HWLOC_INCLUDE_DIR, where to find hwloc.h, etc.
#   HWLOC_LIBRARIES, the libraries needed to use hwloc.
#   HWLOC_FOUND, If false, do not try to use hwloc.
#
# also defined, but not for general use are
#
# ::
#
#   HWLOC_LIBRARY, where to find the hwloc library.

find_path(HWLOC_INCLUDE_DIR hwloc.h
  /usr/include/
  /usr/local/include/
)

set(HWLOC_NAMES hwloc libhwloc)
find_library(HWLOC_LIBRARY NAMES ${HWLOC_NAMES})

# handle the QUIETLY and REQUIRED arguments and set HWLOC_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HWLOC DEFAULT_MSG HWLOC_LIBRARY HWLOC_INCLUDE_DIR)

if(HWLOC_FOUND)
  set(HWLOC_LIBRARIES ${HWLOC_LIBRARY})
endif()

mark_as_advanced(HWLOC_LIBRARY HWLOC_INCLUDE_DIR )
