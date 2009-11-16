TOP        = ../..
SWIG       = $(TOP)/../preinst-swig
SRCS       = example.c
TARGET     = example_wrap
INTERFACE  = example.i
SWIGOPT    =
DSRCS      = *.d
DFLAGS     = -ofrunme

all::	d

d::
	$(MAKE) -f $(TOP)/Makefile SRCS='$(SRCS)' SWIG='$(SWIG)' \
	SWIGOPT='$(SWIGOPT)' TARGET='$(TARGET)' INTERFACE='$(INTERFACE)' d
	$(MAKE) -f $(TOP)/Makefile DSRCS='$(DSRCS)' DFLAGS='$(DFLAGS)' d_compile

clean::
	$(MAKE) -f $(TOP)/Makefile d_clean

check: all
