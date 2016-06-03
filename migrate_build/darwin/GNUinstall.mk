ifndef PREFIX
    PREFIX=/usr
endif

# Incorrect for Mac Os X, this should be easy to fix
install: $(OUT)
	cp $(OUT) $(DESTDIR)$(PREFIX)/lib/libui.0.dylib
	ln -s libui.0.dylib $(DESTDIR)$(PREFIX)/lib/libui.dylib
	cp ui.h ui_$(OS).h $(DESTDIR)$(PREFIX)/include/
