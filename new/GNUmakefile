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
	$(osCFILES:%.c=$(OBJDIR)/%.o)

xCFLAGS = \
	-g \
	-Wall -Wextra \
	-Wno-unused-parameter \
	-Wno-switch \
	--std=c99 \
	-fPIC \
	$(CFLAGS)

xLDFLAGS = \
	-g \
	-fPIC \
	$(LDFLAGS)

OUT = $(OUTDIR)/$(OUTBASE)$(osLIBSUFFIX)

include test/GNUmakeinc.mk

testOFILES = $(testCFILES:%.c=$(OBJ)/test/%.o)

TEST = $(OUTDIR)/$(OUTBASE)$(osEXESUFFIX)

default: $(OUT)
test: $(TEST)
.PHONY: default test

$(OUT): $(OFILES) dirs
	$(CC) -o $(OUT) $(OFILES) -shared $(xLDFLAGS) $(osLDFLAGS)

$(OBJDIR)/%.o: %.c $(HFILES) $(osHFILES) dirs
	$(CC) -o $@ -c $< $(xCFLAGS) $(osCFLAGS)

$(TEST): $(OUT) $(testOFILES) dirs $(OBJDIR)/test/.phony
	$(CC) -o $(TEST) $(testOFILES) $(OUT) $(xLDFLAGS)

$(OBJDIR)/test/%.o: test/%.c ui.h dirs $(OBJDIR)/test/.phony
	$(CC) -o $@ -c $< $(xCFLAGS)

dirs: $(OBJDIR)/.phony $(OBJDIR)/$(OS)/.phony $(OUTDIR)/.phony
.PHONY: dirs

# see http://www.cmcrossroads.com/article/making-directories-gnu-make
%/.phony:
	mkdir -p $(dir $@)
	touch $@

ui.h: ui.idl
	idl2h -extern _UI_EXTERN < ui.idl > ui.h

clean:
	rm -rf $(OUTDIR) $(OBJDIR) ui.h
.PHONY: clean
