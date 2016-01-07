# 16 october 2015

# TODO the loader looks for the soname, not the base name, which is frustrating

# Global flags.

CFLAGS += \
	-fPIC \
	-Wall -Wextra -pedantic \
	-Wno-unused-parameter \
	-Wno-switch \
	--std=c99

# C++11 is needed due to stupid rules involving commas at the end of enum lists that C++03 stupidly didn't follow
# This means sorry, no GCC 2 for Haiku builds :(
CXXFLAGS += \
	-fPIC \
	-Wall -Wextra -pedantic \
	-Wno-unused-parameter \
	-Wno-switch \
	--std=c++11

LDFLAGS += \
	-fPIC

ifneq ($(NODEBUG),1)
	CFLAGS += -g
	CXXFLAGS += -g
	LDFLAGS += -g
endif

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

$(OUT): $(OFILES) | $(OUTDIR)
	@$(reallinker) -o $(OUT) $(OFILES) $(LDFLAGS)
ifeq ($(USESSONAME),1)
	@ln -sf $(NAME)$(SUFFIX) $(OUTNOSONAME)
endif
	@echo ====== Linked $(OUT)

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

# TODO split into $(RC) and $(CVTRES) forms
# with binutils windres can either go straight to a .o file or in the normal two steps
$(OBJDIR)/%.rc.o: $$(subst _,/,%).rc $(HFILES) | $(OBJDIR)
	@$(RC) $(RCFLAGS) $< $@
	@echo ====== Compiled $<

$(OBJDIR) $(OUTDIR):
	@mkdir -p $@
