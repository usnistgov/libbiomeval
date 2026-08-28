# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
#
# Created by NIST for the Biometric Evaluation Framework.
#
#.rst:
# FindLERC
# --------
#
# Find the Lerc headers and library.
#
# ::
#
#   Lerc_* for all vars set by pkg_check_modules
#
# It also sets the target Lerc::Lerc

include(FindPackageHandleStandardArgs)

if(VCPKG_TOOLCHAIN AND PKG_CONFIG_EXECUTABLE)
    # vcpkg seems to include the arglist in PKG_CONFIG_EXECUTABLE.
    # CMake >=3.22 splits these up.
    list(LENGTH PKG_CONFIG_EXECUTABLE _pkgconf_len)
    if(_pkgconf_len GREATER 1)
        list(GET PKG_CONFIG_EXECUTABLE 0 _pkgconf_exe)
        list(SUBLIST PKG_CONFIG_EXECUTABLE 1 -1 _pkgconf_argn)

        set(PKG_CONFIG_EXECUTABLE "${_pkgconf_exe}" CACHE FILEPATH "pkg-config executable" FORCE)
        set(PKG_CONFIG_ARGN "${_pkgconf_argn}" CACHE STRING "Additional pkg-config arguments" FORCE)

        unset(_pkgconf_exe)
        unset(_pkgconf_argn)
    endif()
    unset(_pkgconf_len)

    # vcpkg also sets the executable to where it would install rather than
    # check if it is actually installed, leading to strange errors if missing
    if(PKG_CONFIG_EXECUTABLE AND NOT EXISTS "${PKG_CONFIG_EXECUTABLE}")
        message(VERBOSE "FindLERC: vcpkg-provided pkg-config executable does not exist on disk (${PKG_CONFIG_EXECUTABLE}); ignoring it.")
        set(PKG_CONFIG_EXECUTABLE "PKG_CONFIG_EXECUTABLE-NOTFOUND" CACHE FILEPATH "pkg-config executable" FORCE)
        unset(PKG_CONFIG_ARGN CACHE)
    endif()
endif()

find_package(PkgConfig)
if (PkgConfig_FOUND)
    pkg_check_modules(Lerc IMPORTED_TARGET Lerc)

    if(Lerc_FOUND AND NOT TARGET Lerc::Lerc)
	    add_library(Lerc::Lerc INTERFACE IMPORTED)
	    target_link_libraries(Lerc::Lerc INTERFACE PkgConfig::Lerc)
    endif()
endif()

find_package_handle_standard_args(Lerc DEFAULT_MSG Lerc_FOUND)
