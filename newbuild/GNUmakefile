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
	else ifeq ($(UNAME),Haiku)
		OS = haiku
	else
		OS = unix
	endif
endif

# default is to build with debug symbols
ifndef NODEBUG
	NODEBUG = 0
endif

# TODO $(CC), $(CXX)? if so, $(LD)?
# TODO quotes for the FLAGS options?
ARGS = \
	OS=$(OS) \
	OUTDIR=$(OUTDIR) \
	OBJDIR=$(OBJDIR) \
	CFLAGS=$(CFLAGS) \
	CXXFLAGS=$(CXXFLAGS) \
	LDFLAGS=$(LDFLAGS) \
	NODEBUG=$(NODEBUG) \
	inlibuibuild=1

real:
	@echo $(MAKE)
	@echo $(ARGS)

libui:
	@$(MAKE) -f build/GNUmakefile.libui $(ARGS)

clean:
	rm -rf $(OBJDIR) $(OUTDIR)

test: libui
	@$(MAKE) -f build/GNUmakefile.test $(ARGS)

# TODO provide a build option for the queuemaintest

example: libui
	@$(MAKE) -f build/GNUmakefile.example $(ARGS) EXAMPLE=$(EXAMPLE)

# TODO examples rule?
