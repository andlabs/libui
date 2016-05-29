# 28 may 2016

$(OUT): $(OFILES) | $(OUTDIR)
ifeq (,$(STATICLIB))
	@$(reallinker) -o $(OUT) $(OFILES) $(LDFLAGS)
ifeq ($(USESSONAME),1)
	@ln -sf $(NAME)$(SUFFIX) $(OUTNOSONAME)
endif
else
	nm -m $(OFILES) | sed -E -n 's/^[0-9a-f]* \([A-Z_]+,[a-z_]+\) external //p' > $(OBJDIR)/symbols
	$(LD) -exported_symbols_list $(OBJDIR)/symbols -r $(OFILES) -o $(OUT:%.a=%.o)
	$(AR) rcs $(OUT) $(OUT:%.a=%.o)
endif
	@echo ====== Linked $(OUT)

