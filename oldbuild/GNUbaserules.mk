# 16 october 2015

OFILES = \
	$(subst /,_,$(CFILES)) \
	$(subst /,_,$(CXXFILES)) \
	$(subst /,_,$(MFILES)) \
	$(subst /,_,$(RCFILES))

OFILES := $(OFILES:%=$(OBJDIR)/%.o)

CFLAGS += \
	-g \
	-Wall -Wextra -pedantic \
	-Wno-unused-parameter \
	-Wno-switch \
	--std=c99

# C++11 is needed due to stupid rules involving commas at the end of enum lists that C++03 stupidly didn't follow
# This means sorry, no GCC 2 for Haiku builds :(
CXXFLAGS += \
	-g \
	-Wall -Wextra -pedantic \
	-Wno-unused-parameter \
	-Wno-switch \
	--std=c++11

LDFLAGS += \
	-g

OUT = $(OUTDIR)/$(NAME)$(SUFFIX)

ifdef CXXFILES
	reallinker = $(CXX)
else
	reallinker = $(CC)
endif

$(OUT): $(OFILES) | $(OUTDIR)
	@$(reallinker) -o $(OUT) $(OFILES) $(LDFLAGS)
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

$(OBJDIR)/%.rc.o: $$(subst _,/,%).rc $(HFILES) | $(OBJDIR)
	@$(RC) $(RCFLAGS) $< $@
	@echo ====== Compiled $<

$(OBJDIR) $(OUTDIR):
	@mkdir -p $@

clean:
	rm -rf $(OBJDIR) $(OUTDIR)
.PHONY: clean
