# 2 september 2015

OUT = histogram

ifeq ($(OS),windows)
linkto = ../../out/new.dll
CC = i686-w64-mingw32-gcc
OUT = controlgallery.exe
else
ifeq ($(OS),darwin)
linkto = ../../out/new.dylib
else
linkto = ../../out/new.so
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
