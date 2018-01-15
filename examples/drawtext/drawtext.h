// 20 january 2017
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../ui.h"

struct example {
	const char *name;
	uiControl *panel;
	void (*draw)(uiAreaDrawParams *p);
	void (*mouse)(uiAreaMouseEvent *e);
	int (*key)(uiAreaKeyEvent *e);
	// TODO key?
};

// main.c
extern void redraw(void);

// basic.c
extern struct example *mkBasicExample(void);

// hittest.c
extern struct example *mkHitTestExample(void);

// attributes.c
extern struct example *mkAttributesExample(void);

// emptystr_hittest.c
extern struct example *mkEmptyStringExample(void);
