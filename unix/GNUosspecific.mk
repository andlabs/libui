# 16 october 2015

EXESUFFIX =
LIBSUFFIX = .so
OSHSUFFIX = .h
TOOLCHAIN = gcc

# LONGTERM clean up all the NAMEs and SUFFIXs and NOSOSUFFIXs or whatever it was
USESSONAME = 1
SOVERSION = $(SOVERSION0)
SONAMEEXT = $(LIBSUFFIX).$(SOVERSION)
# this is not gcc-global because OS X uses a different filename format
SONAMEFLAG = -Wl,-soname,
