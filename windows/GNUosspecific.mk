# 16 october 2015

EXESUFFIX = .exe
LIBSUFFIX = .dll
OSHSUFFIX = .h

# TODO only when cross-compiling?
ifeq ($(ARCH),amd64)
	CC = x86_64-w64-mingw32-gcc
	RC = x86_64-w64-mingw32-windres
else ifeq ($(ARCH),386)
	CC = i686-w64-mingw32-gcc
	RC = i686-w64-mingw32-windres
else ifneq ($(ARCH),default)
$(error [FAIL] unknown architecture $(ARCH) for Windows build; I need to know which compiler to invoke)
endif
