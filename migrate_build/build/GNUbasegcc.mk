# 16 october 2015

# Global flags.

CFLAGS += \
	--std=c99

CXXFLAGS += \
	--std=c++11

# Build rules.

OFILES = \
	$(subst /,_,$(CFILES)) \
	$(subst /,_,$(CXXFILES)) \
	$(subst /,_,$(MFILES)) \
	$(subst /,_,$(RCFILES))

OFILES := $(OFILES:%=$(OBJDIR)/%.o)

OUT = $(OUTDIR)/$(NAME)$(SUFFIX)
OUTNOSONAME = $(OUTDIR)/$(NAME)$(LIBSUFFIX)

# TODO allow using LD
# LD is defined by default so we need a way to override the default define without blocking a user define
ifeq ($(CXXFILES),)
	reallinker = $(CC)
else
	reallinker = $(CXX)
endif

include $(OS)/GNUosspecificlink.mk

.SECONDEXPANSION:

$(OBJDIR)/%.c.o: $$(subst _,/,%).c $(HFILES) | $(OBJDIR)
	@$(CC) -o $@ -c $< $(CFLAGS)
	@echo ====== Compiled $<

$(OBJDIR)/%.cpp.o: $$(subst _,/,%).cpp $(HFILES) | $(OBJDIR)
	@$(CXX) -o $@ -c $< $(CXXFLAGS)
	@echo ====== Compiled $<

$(OBJDIR)/%.m.o: $$(subst _,/,%).m $(HFILES) | $(OBJDIR)
	@$(CC) -o $@ -c $< $(CFLAGS)
	@echo ====== Compiled $<

$(OBJDIR)/%.rc.o: $$(subst _,/,%).rc $(HFILES) | $(OBJDIR)
	@$(RC) $(RCFLAGS) $< $@
	@echo ====== Compiled $<

$(OBJDIR) $(OUTDIR):
	@mkdir -p $@
