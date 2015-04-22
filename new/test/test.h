// 22 april 2015
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include "../ui.h"

// main.c
extern void die(const char *, ...);

// spaced.c
extern void setSpaced(int);
extern uiBox *newHorizontalBox(void);
extern uiBox *newVerticalBox(void);
