# 2 september 2015

OUT = histogram

ifeq ($(OS),windows)
linkto = ../../out/libui.dll
CC = i686-w64-mingw32-gcc
ifeq ($(ARCH),amd64)
CC = x86_64-w64-mingw32-gcc
endif
OUT = histogram.exe
else
ifeq ($(OS),darwin)
linkto = ../../out/libui.dylib
else
linkto = ../../out/libui.so
endif
endif

CFILES = \
	main.c

OFILES = $(CFILES:%.c=%.o)

controlgallery: $(OFILES)
	$(CC) -g -o $(OUT) $(OFILES) $(linkto)

%.o: %.c
	$(CC) -c -g -o $@ $<

clean:
	rm -f $(OUT) $(OFILES)
