#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#

# This set of directories is where the header files, libraries, programs,
# and man pages are to be installed.
INCPATH := /usr/local/include
LIBPATH := /usr/local/lib
BINPATH := /usr/local/bin
MANPATH := /usr/local/man/man1

#
# Files and directories that are created during the build process, that
# are to be removed during 'make clean'.
DISPOSABLEFILES = *.o *.exe .gdb_history
DISPOSABLEDIRS  = *.dSYM

CP := cp -f
RM := rm -f
PWD := $(shell pwd)
OS := $(shell uname -s)
ARCH := $(shell uname -m)
ifeq ($(OS),Darwin)
	CC = clang
	CXX = clang++
else
	CC = gcc
	CXX = g++
endif
MPICXX=mpicxx

#
# Determine if CXX can handle C++11 (-std=c++11 flag)
#
CXX_BASENAME := $(shell basename $(CXX))
ifeq ($(CXX_BASENAME),g++)
	CXX_MAJOR := $(shell expr `$(CXX) -dumpversion | cut -f1 -d.`)
	CXX_MINOR := $(shell expr `$(CXX) -dumpversion | cut -f2 -d.`)

#	-std=c++11 first present in G++ 4.7
	SUPPORTS_CXX11 := $(shell [ $(CXX_MAJOR) -ge 5 -o \( $(CXX_MAJOR) -eq 4 -a $(CXX_MINOR) -ge 7 \) ] && echo true)
else
	ifeq ($(CXX_BASENAME),clang++)
		CXX_MAJOR := $(shell $(CXX) --version | tr '\n' ' ' | sed 's/.*version\ \([[:digit:]]\)\.[[:digit:]].*/\1/')
		CXX_MINOR := $(shell $(CXX) --version | tr '\n' ' ' | sed 's/.*version\ [[:digit:]]\.\([[:digit:]]\).*/\1/')

		ifeq ($(OS),Darwin)
#			Apple changes clang++ version to match Xcode,
#			so require Xcode 4.2
			SUPPORTS_CXX11 := $(shell [ $(CXX_MAJOR) -ge 5 -o \( $(CXX_MAJOR) -eq 4 -a $(CXX_MINOR) -ge 2 \) ] && echo true)
		else
#			Require clang++ 3.1
			SUPPORTS_CXX11 := $(shell [ $(CXX_MAJOR) -ge 4 -o \( $(CXX_MAJOR) -eq 3 -a $(CXX_MINOR) -ge 1 \) ] && echo true)
		endif

		EXTRA_CXXFLAGS += -stdlib=libc++
	else
                $(warning CXX=$(CXX) is untested.  I assume you know what you are doing...)
		SUPPORTS_CXX11 := true
	endif
endif
ifneq ($(SUPPORTS_CXX11),true)
        $(error This compiler (CXX=$(CXX)) version does not support -std=c++11)
endif

ifeq ($(findstring CYGWIN,$(OS)), CYGWIN)
        ROOT = Administrator
	OS = CYGWIN
else
        ROOT  = root
endif
ifeq ($(findstring x86_64,$(ARCH)), x86_64)
	ifneq ($(findstring CYGWIN,$(OS)), CYGWIN)
        	ARCHOPT = -fPIC
	endif
endif


#
# If there are any 'non-standard' include or lib directories that need to
# be searched prior to the 'standard' libraries, add the to the CFLAGS
# variable.

CFLAGS := -Wall -pedantic -std=c99 -D$(OS) $(ARCHOPT) $(COMMONINCOPT) -I$(LOCALINC) -I$(INCPATH)
CXXFLAGS := -Wall -pedantic -D$(OS) $(ARCHOPT) $(COMMONINCOPT) -I$(LOCALINC) -I$(INCPATH)
LDFLAGS := -L$(LOCALLIB) -L$(LIBPATH)

# Enable C++11 support.
ifeq ($(findstring CYGWIN,$(OS)), CYGWIN)
	# Cygwin's g++ currently requires the GNU dialect
	CXXFLAGS += -std=gnu++11
	# Cygwin's newlib does not define long double versions of many core C
	# functions, which are ultimately required for C++11's std::to_string,
	# and will instead cast long doubles to doubles when _LDBL_EQ_DBL is 
	# defined.
	CXXFLAGS += -D_GLIBCXX_USE_C99 -D_LDBL_EQ_DBL
else
	CXXFLAGS += -std=c++11
endif

# Enable debugging symbols when DEBUG=1, true, or yes
ifdef DEBUG
	CFLAGS += $(if $(or $(findstring 1, $(DEBUG)), $(findstring true, $(DEBUG))m $(findstring yes, $(DEBUG))),-g)
	CXXFLAGS += $(if $(or $(findstring 1, $(DEBUG)), $(findstring true, $(DEBUG))m $(findstring yes, $(DEBUG))),-g)
endif

# Force 64-bit compilation when 64=1, true, or yes
ifdef 64
	CFLAGS += $(if $(or $(findstring 1, $(64)), $(findstring true, $(64)), $(findstring yes, $(64))),-m64)
	CXXFLAGS += $(if $(or $(findstring 1, $(64)), $(findstring true, $(64)), $(findstring yes, $(64))),-m64)
endif

# Force 32-bit compilation when 32=1, true, or yes
ifdef 32
	CFLAGS += $(if $(or $(findstring 1, $(32)), $(findstring true, $(32)), $(findstring yes, $(32))),-m32)
	CXXFLAGS += $(if $(or $(findstring 1, $(32)), $(findstring true, $(32)), $(findstring yes, $(32))),-m32)
endif

# Add additional flags on build/link without editing this file
ifdef EXTRA_CFLAGS
	CFLAGS += $(EXTRA_CFLAGS)
endif
ifdef EXTRA_CXXFLAGS
	CXXFLAGS += $(EXTRA_CXXFLAGS)
endif
ifdef EXTRA_LDFLAGS
	LDFLAGS += $(EXTRA_LDFLAGS)
endif

