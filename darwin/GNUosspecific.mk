# 16 october 2015

EXESUFFIX =
LIBSUFFIX = .dylib
OSHSUFFIX = .h
STATICLIBSUFFIX = .a
TOOLCHAIN = gcc

USESSONAME = 1
SOVERSION = $(SOVERSIONA)
SONAMEEXT = .$(SOVERSION)$(LIBSUFFIX)
# note the explicit need for @rpath
# LONGTERM -current_version, -compatibility_version
SONAMEFLAG = -Wl,-install_name,@rpath/

NATIVE_UI_LDFLAGS += \
	-lobjc \
	-framework Foundation \
	-framework AppKit

