# 16 october 2015

# IMPORTANT
# Do NOT use / for command-line options here!
# This breaks on GNU makes that come with some versions of
# MinGW because they mangle things that start with /, thinking that
# those arguments are Unix paths that need to be converted to
# Windows paths. This cannot be turned off. -_-'
# MSDN says cl, rc, and link all accept - instead of /, so we're good.
# See also:
# - https://github.com/andlabs/libui/issues/16
# - http://www.mingw.org/wiki/Posix_path_conversion
# - http://www.mingw.org/wiki/FAQ
# - http://stackoverflow.com/questions/7250130/how-to-stop-mingw-and-msys-from-mangling-path-names-given-at-the-command-line
# - http://stackoverflow.com/questions/28533664/how-to-prevent-msys-to-convert-the-file-path-for-an-external-program

# TODO subsystem version

# TODO silence compiler non-diagnostics (/nologo is not enough)

# Global flags.

# TODO /Wall does too much
# TODO -Wno-switch equivalent
# TODO /sdl turns C4996 into an ERROR
# TODO loads of warnings in the system header files
# TODO /analyze requires us to write annotations everywhere
# TODO undecided flags from qo?
# -RTCc is not supplied because it's discouraged as of VS2015; see https://www.reddit.com/r/cpp/comments/46mhne/rtcc_rejects_conformant_code_with_visual_c_2015/d06auq5


# TODO warnings on undefined symbols
LDFLAGS += \
	-largeaddressaware -nologo -incremental:no

ifneq ($(RELEASE),1)
	CFLAGS += -Zi
	CXXFLAGS += -Zi
	LDFLAGS += -debug
endif

# Build rules.

OFILES = \
	$(subst /,_,$(CFILES)) \
	$(subst /,_,$(CXXFILES)) \
	$(subst /,_,$(MFILES))
ifeq (,$(STATIC))
OFILES += \
	$(subst /,_,$(RCFILES))
else
RESFILES = \
	$(subst /,_,$(RCFILES))
endif

OFILES := $(OFILES:%=$(OBJDIR)/%.o)

OUT = $(OUTDIR)/$(NAME)$(SUFFIX)
ifneq (,$(STATIC))
RESOUT = $(OUTDIR)/$(NAME).res
endif
# otherwise keep $(RESOUT) empty

# TODO use $(CC), $(CXX), $(LD), and s$(RC)

$(OUT): $(OFILES) $(RESOUT) | $(OUTDIR)
ifeq (,$(STATICLIB))
	@link -out:$(OUT) $(OFILES) $(LDFLAGS)
else
	@lib -out:$(OUT) $(OFILES)
endif
	@echo ====== Linked $(OUT)

.SECONDEXPANSION:

# TODO can we put /Fd$@.pdb in a variable?
$(OBJDIR)/%.c.o: $$(subst _,/,%).c $(HFILES) | $(OBJDIR)
ifeq ($(RELEASE),1)
	@cl -Fo:$@ -c $< $(CFLAGS)
else
	@cl -Fo:$@ -c $< $(CFLAGS) -Fd$@.pdb
endif
	@echo ====== Compiled $<

$(OBJDIR)/%.cpp.o: $$(subst _,/,%).cpp $(HFILES) | $(OBJDIR)
ifeq ($(RELEASE),1)
	@cl -Fo:$@ -c $< $(CXXFLAGS)
else
	@cl -Fo:$@ -c $< $(CXXFLAGS) -Fd$@.pdb
endif
	@echo ====== Compiled $<

# note: don't run cvtres directly; the linker does that for us
$(OBJDIR)/%.rc.o: $$(subst _,/,%).rc $(HFILES) | $(OBJDIR)
	@rc -nologo -v -fo $@ $(RCFLAGS) $<
	@echo ====== Compiled $<
$(RESOUT): $$(RCFILES) $(HFILES) | $(OUTDIR)
	@rc -nologo -v -fo $@ $(RCFLAGS) $<
	@echo ====== Compiled $<

$(OBJDIR) $(OUTDIR):
	@mkdir $@
