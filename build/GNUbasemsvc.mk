# 16 october 2015

# TODO subsystem version

# TODO silence compiler non-diagnostics (/nologo is not enough)

# Global flags.

# TODO /Wall does too much
# TODO -Wno-switch equivalent
# TODO /sdl turns C4996 into an ERROR
# TODO loads of warnings in the system header files
# TODO /analyze requires us to write annotations everywhere
# TODO undecided flags from qo?
CFLAGS += \
	/W4 \
	/wd4100 \
	/TC \
	/bigobj /nologo \
	/RTC1 /RTCc /RTCs /RTCu

CXXFLAGS += \
	/W4 \
	/wd4100 \
	/TP \
	/bigobj /nologo \
	/RTC1 /RTCc /RTCs /RTCu

# TODO warnings on undefined symbols
LDFLAGS += \
	/largeaddressaware /nologo /incremental:no

ifneq ($(NODEBUG),1)
	CFLAGS += /Zi
	CXXFLAGS += /Zi
	LDFLAGS += /debug
endif

# Build rules.

OFILES = \
	$(subst /,_,$(CFILES)) \
	$(subst /,_,$(CXXFILES)) \
	$(subst /,_,$(MFILES)) \
	$(subst /,_,$(RCFILES))

OFILES := $(OFILES:%=$(OBJDIR)/%.o)

OUT = $(OUTDIR)/$(NAME)$(SUFFIX)

# TODO use $(CC), $(CXX), $(LD), $(RC), and $(CVTRES)

$(OUT): $(OFILES) | $(OUTDIR)
	@link /out:$(OUT) $(OFILES) $(LDFLAGS)
	@echo ====== Linked $(OUT)

.SECONDEXPANSION:

# TODO can we put /Fd$@.pdb in a variable?
$(OBJDIR)/%.c.o: $$(subst _,/,%).c $(HFILES) | $(OBJDIR)
ifeq ($(NODEBUG),1)
	@cl /Fo:$@ /c $< $(CFLAGS)
else
	@cl /Fo:$@ /c $< $(CFLAGS) /Fd$@.pdb
endif
	@echo ====== Compiled $<

$(OBJDIR)/%.cpp.o: $$(subst _,/,%).cpp $(HFILES) | $(OBJDIR)
ifeq ($(NODEBUG),1)
	@cl /Fo:$@ /c $< $(CXXFLAGS)
else
	@cl /Fo:$@ /c $< $(CXXFLAGS) /Fd$@.pdb
endif
	@echo ====== Compiled $<

$(OBJDIR)/%.rc.o: $$(subst _,/,%).rc $(HFILES) | $(OBJDIR)
	@rc /nologo /v /fo $@.res $<
	@cvtres /nologo /out:$@ $@.res
	@echo ====== Compiled $<

$(OBJDIR) $(OUTDIR):
	@mkdir $@
