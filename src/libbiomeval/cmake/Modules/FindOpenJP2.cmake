# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
#
# Created by NIST for the Biometric Evaluation Framework.
#
#.rst:
# FindOpenJP2
# --------
#
# Find OpenJPEG 2.x
#
# Find the Open JPEG 2.x includes and library This module defines
#
# ::
#
#   OPENJP2_INCLUDE_DIR, where to find openjpeg.h, etc.
#   OPENJP2_LIBRARIES, the libraries needed to use JP2.
#   OPENJP2_FOUND, If false, do not try to use JP2.
#
# also defined, but not for general use are
#
# ::
#
#   OPENJP2_LIBRARY, where to find the OpenJPEG library.

find_path(OPENJP2_INCLUDE_DIR opj_config.h
  /usr/local/include/openjpeg-2.1/
  /opt/local/include/openjpeg-2.1/
  /usr/local/include/openjpeg-2.2/
  /opt/local/include/openjpeg-2.2/
)

set(OPENJP2_NAMES ${OPENJP2_NAMES} openjp2 libopenjp2)
find_library(OPENJP2_LIBRARY NAMES ${OPENJP2_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set OPENJP2_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENJP2 DEFAULT_MSG OPENJP2_LIBRARY OPENJP2_INCLUDE_DIR)

if(OPENJP2_FOUND)
  set(OPENJP2_LIBRARIES ${OPENJP2_LIBRARY})
endif()

mark_as_advanced(OPENJP2_LIBRARY OPENJP2_INCLUDE_DIR )
