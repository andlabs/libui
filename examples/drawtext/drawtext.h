// 20 january 2017
#include <stdio.h>
#include <string.h>
#include "../../ui.h"

struct example {
	const char *name;
	uiControl *panel;
	void (*draw)(uiAreaDrawParams *p);
	void (*mouse)(uiAreaMouseEvent *e);
	// TODO key?
};

// main.c
extern void redraw(void);

// basic.c
extern struct example *mkBasicExample(void);

// hittest.c
extern struct example *mkHitTestExample(void);
