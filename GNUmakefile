# 16 october 2015

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
	else
		OS = unix
	endif
endif

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
	@$(MAKE) -f GNUmakefile.libui OS=$(OS) ARCH=$(ARCH) OUTDIR=$(OUTDIR) OBJDIR=$(OBJDIR)

clean:
	@$(MAKE) -f GNUmakefile.libui OS=$(OS) ARCH=$(ARCH) OUTDIR=$(OUTDIR) OBJDIR=$(OBJDIR) clean

test: libui
	@$(MAKE) -f GNUmakefile.test OS=$(OS) ARCH=$(ARCH) OUTDIR=$(OUTDIR) OBJDIR=$(OBJDIR)
