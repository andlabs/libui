// 22 april 2015
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include "../out/ui.h"

// main.c
extern void die(const char *, ...);
extern uiBox *mainBox;
extern uiTab *mainTab;

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
extern uiBox *makePage2(void);

// page3.c
extern uiBox *makePage3(void);
