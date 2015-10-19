// 22 april 2015
// TODO
#define _GNU_SOURCE
#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "../ui.h"

// main.c
extern void die(const char *, ...);
extern uiBox *mainBox;
extern uiTab *mainTab;
extern uiBox *(*newhbox)(void);
extern uiBox *(*newvbox)(void);

// spaced.c
extern void setSpaced(int);
extern void querySpaced(char[12]);
extern uiWindow *newWindow(const char *title, int width, int height, int hasMenubar);
extern uiBox *newHorizontalBox(void);
extern uiBox *newVerticalBox(void);
extern uiTab *newTab(void);
extern uiGroup *newGroup(const char *);

// menus.c
extern uiMenuItem *shouldQuitItem;
extern void initMenus(void);

// page1.c
extern uiBox *page1;
extern void makePage1(uiWindow *);

// page2.c
extern uiGroup *page2group;
extern uiBox *makePage2(void);

// page3.c
extern uiBox *makePage3(void);

// page4.c
extern uiBox *makePage4(void);

// page5.c
extern uiBox *makePage5(void);

// page6.c
extern uiBox *makePage6(void);

// drawtests.c
extern void runDrawTest(intmax_t, uiAreaDrawParams *);
extern void populateComboboxWithTests(uiCombobox *);

// page7.c
extern uiBox *makePage7(void);

// page8.c
extern uiBox *makePage8(void);
