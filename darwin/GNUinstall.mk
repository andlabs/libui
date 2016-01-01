ifndef PREFIX
    PREFIX=/usr
endif

# Incorrect for Mac Os X, this should be easy to fix
install: $(OUT)
	cp $(OUT) $(DESTDIR)$(PREFIX)/lib/libui.0.dylib
	ln -s libui.so.0 $(DESTDIR)$(PREFIX)/lib/libui.so
	cp ui.h ui_$(OS).h $(DESTDIR)$(PREFIX)/include/
