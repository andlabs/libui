typedef struct uiMenuItem uiMenuItem;
#define uiMenuItem(this) ((uiMenuItem *) (this))
uiprivExtern void uiMenuItemEnable(uiMenuItem *m);
uiprivExtern void uiMenuItemDisable(uiMenuItem *m);
uiprivExtern void uiMenuItemOnClicked(uiMenuItem *m, void (*f)(uiMenuItem *sender, uiWindow *window, void *data), void *data);
uiprivExtern int uiMenuItemChecked(uiMenuItem *m);
uiprivExtern void uiMenuItemSetChecked(uiMenuItem *m, int checked);

typedef struct uiMenu uiMenu;
#define uiMenu(this) ((uiMenu *) (this))
uiprivExtern uiMenuItem *uiMenuAppendItem(uiMenu *m, const char *name);
uiprivExtern uiMenuItem *uiMenuAppendCheckItem(uiMenu *m, const char *name);
uiprivExtern uiMenuItem *uiMenuAppendQuitItem(uiMenu *m);
uiprivExtern uiMenuItem *uiMenuAppendPreferencesItem(uiMenu *m);
uiprivExtern uiMenuItem *uiMenuAppendAboutItem(uiMenu *m);
uiprivExtern void uiMenuAppendSeparator(uiMenu *m);
uiprivExtern uiMenu *uiNewMenu(const char *name);
