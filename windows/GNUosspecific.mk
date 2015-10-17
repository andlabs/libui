# 16 october 2015

EXESUFFIX = .exe
LIBSUFFIX = .dll

ifeq ($(ARCH),amd64)
	ifndef CC
		CC = x86_64-w64-mingw32-gcc
	endif
	ifndef RC
		RC = x86_64-w64-mingw32-windres
	endif
else ifeq ($(ARCH),386)
	ifndef CC
		CC = i686-w64-mingw32-gcc
	endif
	ifndef RC
		RC = i686-w64-mingw32-windres
	endif
else ifneq ($(ARCH),default)
	$(error [FAIL] unknown architecture $(ARCH))
endif
