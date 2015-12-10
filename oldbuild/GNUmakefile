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

libui:
	@$(MAKE) -f GNUmakefile.libui OS=$(OS) OUTDIR=$(OUTDIR) OBJDIR=$(OBJDIR) inlibuibuild=1

# TODO why is this not a regular old rule
clean:
	@$(MAKE) -f GNUmakefile.libui OS=$(OS) OUTDIR=$(OUTDIR) OBJDIR=$(OBJDIR) inlibuibuild=1 clean

test: libui
	@$(MAKE) -f GNUmakefile.test OS=$(OS) OUTDIR=$(OUTDIR) OBJDIR=$(OBJDIR) inlibuibuild=1
