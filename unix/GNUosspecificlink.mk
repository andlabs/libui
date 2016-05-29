# 28 may 2016

$(OUT): $(OFILES) | $(OUTDIR)
ifeq (,$(STATICLIB))
	@$(reallinker) -o $(OUT) $(OFILES) $(LDFLAGS)
ifeq ($(USESSONAME),1)
	@ln -sf $(NAME)$(SUFFIX) $(OUTNOSONAME)
endif
else
	$(LD) -r $(OFILES) -o $(OUT:%.a=%.o)
	objcopy --localize-hidden $(OUT:%.a=%.o)
	$(AR) rcs $(OUT) $(OUT:%.a=%.o)
endif
	@echo ====== Linked $(OUT)

