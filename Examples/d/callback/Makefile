TOP        = ../..
SWIG       = $(TOP)/../preinst-swig
CXXSRCS    = example.cxx
TARGET     = example_wrap
INTERFACE  = example.i
SWIGOPT    =
DSRCS      = *.d
DFLAGS     = -ofrunme

all::	d

d::
	$(MAKE) -f $(TOP)/Makefile CXXSRCS='$(CXXSRCS)' SWIG='$(SWIG)' \
	SWIGOPT='$(SWIGOPT)' TARGET='$(TARGET)' INTERFACE='$(INTERFACE)' d_cpp
	$(MAKE) -f $(TOP)/Makefile DSRCS='$(DSRCS)' DFLAGS='$(DFLAGS)' d_compile

clean::
	$(MAKE) -f $(TOP)/Makefile d_clean

check: all
