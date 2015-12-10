# 16 october 2015

EXESUFFIX =
LIBSUFFIX = .so
OSHSUFFIX = .hpp

# Force GCC 4; GCC 2 is not supported.
gccver = $(shell $(CC) --version | sed 's/-.*//g')
ifeq ($(gccver),2.95.3)
	# TODO warn?
	CC = gcc-x86
	CXX = g++-x86
endif
