ifndef PREFIX
    PREFIX=/usr
endif

install: $(OUT)
	cp $(OUT) $(DESTDIR)$(PREFIX)/lib/libui.so.0
	ln -fs libui.so.0 $(DESTDIR)$(PREFIX)/lib/libui.so
	cp ui.h ui_$(OS).h $(DESTDIR)$(PREFIX)/include/
