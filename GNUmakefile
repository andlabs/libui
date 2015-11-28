# 16 october 2015

# TODO warn on MinGW-w64 builds that lack of isolation awareness means no theming

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

# TODO this is wrong for Haiku
ifndef ARCH
	UNAME = $(shell uname -m)
	ifeq ($(UNAME),x86_64)
		ARCH = amd64
	else ifeq ($(UNAME),i686)
		ARCH = 386
	else
		ARCH = default
	endif
endif

libui:
	@$(MAKE) -f GNUmakefile.libui OS=$(OS) ARCH=$(ARCH) OUTDIR=$(OUTDIR) OBJDIR=$(OBJDIR) inlibuibuild=1

clean:
	@$(MAKE) -f GNUmakefile.libui OS=$(OS) ARCH=$(ARCH) OUTDIR=$(OUTDIR) OBJDIR=$(OBJDIR) inlibuibuild=1 clean

test: libui
	@$(MAKE) -f GNUmakefile.test OS=$(OS) ARCH=$(ARCH) OUTDIR=$(OUTDIR) OBJDIR=$(OBJDIR) inlibuibuild=1
