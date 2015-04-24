# 22 april 2015

OUTBASE = new
OUTDIR = out
OBJDIR = .obj

xHFILES = \
	ui.h \
	$(baseHFILES)

OFILES = \
	$(baseCFILES:%.c=$(OBJDIR)/%.o) \
	$(baseMFILES:%.m=$(OBJDIR)/%.o)

xCFLAGS = \
	-g \
	-Wall -Wextra \
	-Wno-unused-parameter \
	-Wno-switch \
	--std=c99 \
	$(CFLAGS) \
	$(baseCFLAGS)

xLDFLAGS = \
	-g \
	$(LDFLAGS) \
	$(baseLDFLAGS)

OUT = $(OUTDIR)/$(OUTBASE)$(baseSUFFIX)

$(OUT): $(OFILES) | $(OUTDIR)/.phony
	$(CC) -o $(OUT) $(OFILES) $(xLDFLAGS)

.SECONDEXPANSION:
$(OBJDIR)/%.o: %.c $(xHFILES) | $$(dir $$@).phony
	$(CC) -o $@ -c $< $(xCFLAGS)

# see http://www.cmcrossroads.com/article/making-directories-gnu-make
%/.phony:
	mkdir -p $(dir $@)
	touch $@
.PRECIOUS: %/.phony

ui.h: ui.idl
	idl2h -extern _UI_EXTERN < ui.idl > ui.h

clean:
	rm -rf $(OUTDIR) $(OBJDIR) ui.h
.PHONY: clean
