# 15 april 2015

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

CFILES = \
	stack.c \
	test.c
HFILES = \
	ui.h \
	uipriv.h \
	ui_$(OS).h \
	$(OS)/uipriv_$(OS).h

xCFLAGS = \
	-g \
	-Wall -Wextra \
	-Wno-unused-parameter \
	--std=c99 \
	$(CFLAGS)
xLDFLAGS = \
	-g \
	$(LDFLAGS)

include $(OS)/GNUmakeinc.mk
xOSCFILES = $(OSCFILES:%=$(OS)/%)
xOSMFILES = $(OSMFILES:%=$(OS)/%)

OFILES = $(CFILES:%.c=$(OBJDIR)/%.o) \
	$(xOSCFILES:$(OS)/%.c=$(OBJDIR)/%_$(OS).o) \
	$(xOSMFILES:$(OS)/%.m=$(OBJDIR)/%_$(OS).o)

$(OUT): $(OFILES)
	$(CC) -o $(OUT) $(OFILES) $(xLDFLAGS)

$(OBJDIR)/%.o: %.c $(OBJDIR) $(HFILES)
	$(CC) -o $@ -c $< $(xCFLAGS)

$(OBJDIR)/%_$(OS).o: $(OS)/%.c $(OBJDIR) $(HFILES)
	$(CC) -o $@ -c $< $(xCFLAGS)

$(OBJDIR)/%_$(OS).o: $(OS)/%.m $(OBJDIR) $(HFILES)
	$(CC) -o $@ -c $< $(xCFLAGS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

ui.h: ui.idl
	idl2h < ui.idl > ui.h
