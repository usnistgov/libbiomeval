# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
#
# Created by NIST for the Biometric Evaluation Framework.
#
#.rst:
# FindFFMPEG
# --------
#
# Find FFMPEG
#
# Find the FFMPEG includes and library This module defines
#
# ::
#
#   FFMPEG_INCLUDE_DIR, where to find libavcodec/avcodec.h, etc.
#   FFMPEG_LIBRARIES, the libraries needed to use FFMPEG.
#   FFMPEG_FOUND, If false, do not try to use FFMPEG.

# Assume that if one header file is found, they are all present
find_path(FFMPEG_INCLUDE_DIR libavcodec/avcodec.h
  /usr/local/include/
  /opt/local/include/
)

find_library(FFMPEG_AVFORMAT NAMES avformat)
find_library(FFMPEG_AVUTIL NAMES avutil)
find_library(FFMPEG_SWSCALE NAMES swscale)
find_library(FFMPEG_AVCODEC NAMES avcodec)
set(FFMPEG_NAMES FFMPEG_AVCODEC FFMPEG_AVUTIL FFMPEG_SWSCALE FFMPEG_AVCODEC)

# handle the QUIETLY and REQUIRED arguments and set FFMPEG_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FFMPEG DEFAULT_MSG ${FFMPEG_NAMES} FFMPEG_INCLUDE_DIR)

if(FFMPEG_FOUND)
  set(FFMPEG_LIBRARIES ${FFMPEG_AVFORMAT} ${FFMPEG_AVUTIL} ${FFMPEG_SWSCALE} ${FFMPEG_AVCODEC})
endif()
