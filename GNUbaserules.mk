# 16 october 2015

OFILES = \
	$(subst /,_,$(CFILES)) \
	$(subst /,_,$(CXXFILES)) \
	$(subst /,_,$(MFILES)) \
	$(subst /,_,$(RCFILES))

OFILES := $(OFILES:%=$(OBJDIR)/%.o)

CFLAGS += \
	-g \
	-Wall -Wextra \
	-Wno-unused-parameter \
	-Wno-switch \
	--std=c99

# TODO consider switching to C++11
# this will hopelessly disbar Haiku for GCC 2
CXXFLAGS += \
	-g \
	-Wall -Wextra \
	-Wno-unused-parameter \
	-Wno-switch \
	--std=c++03

LDFLAGS += \
	-g

ifeq ($(ARCH),386)
	CFLAGS += -m32
	LDFLAGS += -m32
else ifeq ($(ARCH),amd64)
	CFLAGS += -m64
	LDFLAGS += -m64
endif

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
