// 6 april 2015
#include "ui.h"

typedef struct uiSize uiSize;
typedef struct uiSizing uiSizing;

struct uiSize {
	intmax_t width;
	intmax_t height;
};

struct uiControl {
	uintptr_t (*handle)(uiControl *);
	void (*setParent)(uiControl *, uintptr_t);
	uiSize (*preferredSize)(uiControl *, uiSizing *);
	void (*resizing)(uiControl *, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
	void (*containerShow)(uiControl *);
	void (*containerHide)(uiControl *);
};
