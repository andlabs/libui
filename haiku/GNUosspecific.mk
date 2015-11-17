# 16 october 2015

EXESUFFIX =
LIBSUFFIX = .so
OSHSUFFIX = .hpp

gccver = $(shell $(CC) --version | sed 's/-.*//g')
ifeq ($(gccver),2.95.3)
$(error Sorry; GCC 4 is necessary to build libui for Haiku)
endif
