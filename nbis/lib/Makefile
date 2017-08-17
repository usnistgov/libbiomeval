#*******************************************************************************
#
# License: 
# This software and/or related materials was developed at the National Institute
# of Standards and Technology (NIST) by employees of the Federal Government
# in the course of their official duties. Pursuant to title 17 Section 105
# of the United States Code, this software is not subject to copyright
# protection and is in the public domain. 
#
# This software and/or related materials have been determined to be not subject
# to the EAR (see Part 734.3 of the EAR for exact details) because it is
# a publicly available technology and software, and is freely distributed
# to any interested party with no licensing requirements.  Therefore, it is 
# permissible to distribute this software as a free download from the internet.
#
# Disclaimer: 
# This software and/or related materials was developed to promote biometric
# standards and biometric technology testing for the Federal Government
# in accordance with the USA PATRIOT Act and the Enhanced Border Security
# and Visa Entry Reform Act. Specific hardware and software products identified
# in this software were used in order to perform the software development.
# In no case does such identification imply recommendation or endorsement
# by the National Institute of Standards and Technology, nor does it imply that
# the products and equipment identified are necessarily the best available
# for the purpose.
#
# This software and/or related materials are provided "AS-IS" without warranty
# of any kind including NO WARRANTY OF PERFORMANCE, MERCHANTABILITY,
# NO WARRANTY OF NON-INFRINGEMENT OF ANY 3RD PARTY INTELLECTUAL PROPERTY
# or FITNESS FOR A PARTICULAR PURPOSE or for any purpose whatsoever, for the
# licensed product, however used. In no event shall NIST be liable for any
# damages and/or costs, including but not limited to incidental or consequential
# damages of any kind, including economic damage or injury to property and lost
# profits, regardless of whether NIST shall be advised, have reason to know,
# or in fact shall know of the possibility.
#
# By using this software, you agree to bear all risk relating to quality,
# use and performance of the software and/or related materials.  You agree
# to hold the Government harmless from any claim arising from your use
# of the software.
#
#*******************************************************************************

# ******************************************************************************
#
# Makefile to build library libnbis.
#
# ******************************************************************************
#
LIBDIR = ../../lib
INCPATH := /opt/local/include
LOCALINC := ../include
AR := ar ru
BECHECK = ../big_endian_check
include ../../common.mk
LIBRARY	:= $(NBISEVALFILE)

#
SUBDIRS = wsq jpegb jpegl an2k

WSQSOURCES := $(wildcard wsq/*.c)
JPEGBSOURCES := $(wildcard jpegb/*.c)
JPEGLSOURCES := $(wildcard jpegl/*.c)
AN2KSOURCES := $(wildcard an2k/*.c)
#
BASESOURCES := $(wildcard *.c)
SUBDIRSOURCES := $(WSQSOURCES) $(JPEGBSOURCES) $(JPEGLSOURCES) $(AN2KSOURCES)

WSQOBJFILES := $(WSQSOURCES:%.c=%.o)
JPEGBOBJFILES := $(JPEGBSOURCES:%.c=%.o)
JPEGLOBJFILES := $(JPEGLSOURCES:%.c=%.o)
AN2KOBJFILES := $(AN2KSOURCES:%.c=%.o)
#
WSQRENAMEDOBJS := $(WSQOBJFILES:%.o=%_wsq.o)
JPEGBRENAMEDOBJS := $(JPEGBOBJFILES:%.o=%_jpegb.o)
JPEGLRENAMEDOBJS := $(JPEGLOBJFILES:%.o=%_jpegl.o)
AN2KRENAMEDOBJS := $(AN2KOBJFILES:%.o=%_an2k.o)
SUBDIRRENAMEDOBJS := $(WSQRENAMEDOBJS) $(JPEGBRENAMEDOBJS) $(JPEGLRENAMEDOBJS) $(AN2KRENAMEDOBJS)
#
BASEOBJFILES := $(BASESOURCES:%.c=%.o)
SUBDIROBJFILES := $(WSQOBJFILES) $(JPEGBOBJFILES) $(JPEGLOBJFILES) $(AN2KOBJFILES)
OBJFILES := $(BASEOBJFILES) $(SUBDIROBJFILES)

all: $(LIBRARY)
	test -d $(LIBDIR) || mkdir $(LIBDIR)
	$(CP) $(LIBRARY) $(LIBDIR)
#
$(LIBRARY): becheck $(BASEOBJFILES) $(SUBDIRRENAMEDOBJS)
	$(AR) $@ $(BASEOBJFILES) $(SUBDIRRENAMEDOBJS)

$(SUBDIROBJFILES): $(CFLAGS) += -I../$(LOCALHDRS)
$(SUBDIROBJFILES): $(SUBDIRSOURCES)

# Rename objects files in subdirectories or they will clash with each other
$(SUBDIRRENAMEDOBJS): $(SUBDIROBJFILES)
	$(shell mv $(@D)/$(shell basename $(@F) _$(@D).o).o $@)

ifeq ($(OS), Darwin)
# Requires jpeglib.h from MacPorts on OS X
an2k/decode.o: CFLAGS += -I/opt/local/include
jpegb/decoder.o: CFLAGS += -I/opt/local/include
jpegb/encoder.o: CFLAGS += -I/opt/local/include
jpegb/marker.o: CFLAGS += -I/opt/local/include
jpegb/membuf.o: CFLAGS += -I/opt/local/include
jpegb/ppi.o: CFLAGS += -I/opt/local/include
endif

.PHONY: becheck
becheck: 
ifeq ($(wildcard $(BECHECK)),)
	$(error Can not find $(BECHECK))
else
ifeq ($(shell $(BECHECK)),0)
	$(eval CFLAGS += -D__NBISLE__)
endif
endif

clean:
	$(RM) $(LIBRARY) *.o
	@for subdir in $(SUBDIRS); do \
		(cd $$subdir && $(RM) *.o) \
	done
