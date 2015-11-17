# 16 october 2015

EXESUFFIX =
LIBSUFFIX = .so
OSHSUFFIX = .hpp

# TODO dynamically select the gcc4 compiler instead? gotta figure out how to do this on a GCC 2 Hybrid nightly
gccver = $(shell $(CC) --version | sed 's/-.*//g')
ifeq ($(gccver),2.95.3)
$(error Sorry; GCC 4 is necessary to build libui for Haiku)
endif
