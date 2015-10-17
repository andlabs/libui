# 16 october 2015

OFILES = \
	$(subst /,_,$(CFILES)) \
	$(subst /,_,$(MFILES)) \
	$(subst /,_,$(RCFILES))

OFILES := $(OFILES:%=$(OBJDIR)/%.o)

CFLAGS += \
	-g \
	-Wall -Wextra \
	-Wno-unused-parameter \
	-Wno-switch \
	--std=c99

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

$(OUT): $(OFILES) | $(OUTDIR)
	@$(CC) -o $(OUT) $(OFILES) $(LDFLAGS)
	@echo ====== Linked $(OUT)

.SECONDEXPANSION:

$(OBJDIR)/%.c.o: $$(subst _,/,%).c $(HFILES) | $(OBJDIR)
	@$(CC) -o $@ -c $< $(CFLAGS)
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
