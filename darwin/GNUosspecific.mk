# 16 october 2015

EXESUFFIX =
LIBSUFFIX = .dylib
OSHSUFFIX = .h
TOOLCHAIN = gcc

USESSONAME = 1
SOVERSION = $(SOVERSIONA)
SONAMEEXT = .$(SOVERSION)$(LIBSUFFIX)
# note the explicit need for @rpath
# TODO -current_version, -compatibility_version
SONAMEFLAG = -Wl,-install_name,@rpath/
