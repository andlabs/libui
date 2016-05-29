# 16 october 2015

EXESUFFIX =
LIBSUFFIX = .so
OSHSUFFIX = .h
STATICLIBSUFFIX = .a
TOOLCHAIN = gcc

# LONGTERM clean up all the NAMEs and SUFFIXs and NOSOSUFFIXs or whatever it was
USESSONAME = 1
SOVERSION = $(SOVERSION0)
SONAMEEXT = $(LIBSUFFIX).$(SOVERSION)
# this is not gcc-global because OS X uses a different filename format
SONAMEFLAG = -Wl,-soname,

NATIVE_UI_CFLAGS += \
	`pkg-config --cflags Qt5Widgets`
NATIVE_UI_CXXFLAGS += \
	`pkg-config --cflags Qt5Widgets`
NATIVE_UI_LDFLAGS += \
	`pkg-config --libs Qt5Widgets` -lm
