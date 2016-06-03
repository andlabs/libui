# 16 october 2015

# TODO http://stackoverflow.com/questions/4122831/disable-make-builtin-rules-and-variables-from-inside-the-make-file

# silence entering/leaving messages
MAKEFLAGS += --no-print-directory

OUTDIR = out
OBJDIR = .obj

# MAME does this so :/
ifeq ($(OS),Windows_NT)
	OS = windows
endif

ifndef OS
	UNAME = $(shell uname -s)
	ifeq ($(UNAME),Darwin)
		OS = darwin
	else ifeq ($(UNAME),Haiku)
		OS = haiku
	else
		OS = unix
	endif
endif

# default is to build with debug symbols
ifndef RELEASE
	RELEASE = 0
endif

# parameters
export OS
# TODO CC, CXX, RC, LD
export CFLAGS
export CXXFLAGS
# TODO RCFLAGS
export LDFLAGS
export RELEASE
export EXAMPLE
export PREFIX

# for Debian - see https://github.com/andlabs/libui/pull/10
export DESTDIR

# other important variables
export OBJDIR
export OUTDIR
export STATIC

ifneq (,$(STATIC))
STATICLIB = STATICLIB=1
endif

libui:
	@$(MAKE) -f build/GNUmakefile.libui inlibuibuild=1 $(STATICLIB)

clean:
	rm -rf $(OBJDIR) $(OUTDIR)

test: libui
	@$(MAKE) -f build/GNUmakefile.test inlibuibuild=1

# TODO provide a build option for the queuemaintest

example: libui
	@$(MAKE) -f build/GNUmakefile.example inlibuibuild=1

examples:
	@$(MAKE) -f GNUmakefile example EXAMPLE=controlgallery 
	@$(MAKE) -f GNUmakefile example EXAMPLE=histogram
	@$(MAKE) -f GNUmakefile example EXAMPLE=cpp-multithread

.PHONY: examples

install:
	@$(MAKE) -f build/GNUmakefile.libui install inlibuibuild=1
