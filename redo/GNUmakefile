# 22 april 2015

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

baseHFILES = \
	ui.h \
	uipriv.h \
	ui_$(OS).h \
	$(osHFILES)

baseCFILES = \
	control.c \
	menu.c \
	ptrarray.c \
	shouldquit.c \
	types.c \
	$(osCFILES)

baseMFILES = $(osMFILES)

baseRCFILES = $(osRCFILES)

baseCFLAGS = $(osCFLAGS)
baseLDFLAGS = \
	-shared \
	$(osLDWarnUndefinedFlags) \
	$(osLDFLAGS)
baseRCFLAGS = $(osRCFLAGS)
baseSUFFIX = $(osLIBSUFFIX)

include GNUbase.mk

test: $(OUT)
	@$(MAKE) -f GNUmaketest.mk osLIB=$(OUT) osEXESUFFIX=$(osEXESUFFIX) CC=$(CC) archmflag=$(archmflag)
.PHONY: test
