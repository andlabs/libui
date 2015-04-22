# 22 april 2015

OUTBASE = new
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

include $(OS)/GNUmakeinc.mk

HFILES = \
	ui.h \
	uipriv.h \
	ui_$(OS).h

OFILES = \
	$(CFILES:%.c=$(OBJDIR)/%.o) \
	$(osCFILES:%.c=$(OBJDIR)/%_$(OS).o)

xCFLAGS = \
	-g \
	-Wall -Wextra \
	-Wno-unused-parameter \
	-Wno-switch \
	--std=c99 \
	$(CFLAGS)

xLDFLAGS = \
	-g \
	$(LDFLAGS)

OUT = $(OUTDIR)/$(OUTBASE)$(osLIBSUFFIX)
TEST = $(OUTDIR)/$(OUTBASE)$(osEXESUFFIX)

default: $(OUT)
test: $(TEST)
.PHONY: default test

$(OUT): $(OFILES) $(OUTDIR)
	$(CC) -o $(OUT) $(OFILES) -shared $(xLDFLAGS) $(osLDFLAGS)

$(OBJDIR)/%.o: %.c $(HFILES) $(OBJDIR)
	$(CC) -o $@ -c $< $(xCFLAGS) $(osCFLAGS)

$(OBJDIR)/%_$(OS).o: $(OS)/%.c $(HFILES) $(osHFILES) $(OBJDIR)
	$(CC) -o $@ -c $< $(xCFLAGS) $(osCFLAGS)

$(TEST): $(OUT) $(TESTOFILES) $(OUTDIR)
	$(CC) -o $(TEST) $(TESTOFILES) $(OUT) $(xLDFLAGS)

$(OBJDIR)/%_test.o: test/%.c ui.h $(OBJDIR)
	$(CC) -o $@ -c $< $(xCFLAGS)

# see http://www.cmcrossroads.com/article/making-directories-gnu-make
#$(OUTDIR): $(OUTDIR)/phony
#$(OBJDIR): $(OBJDIR)/phony
#%/phony:
#	mkdir -p $(dir $@)
#	touch $@
#.PRECIOUS: $(OUTDIR) $(OUTDIR)/phony
#.PRECIOUS: $(OBJDIR) $(OBJDIR)/phony
$(OUTDIR) $(OBJDIR):
	mkdir -p $@

ui.h: ui.idl
	idl2h -extern _UI_EXTERN < ui.idl > ui.h

clean:
	rm -rf $(OUTDIR) $(OBJDIR) ui.h
