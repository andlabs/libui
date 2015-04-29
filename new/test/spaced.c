// 22 april 2015
#include "test.h"

struct thing {
	void *ptr;
	int type;
};

static struct thing *things = NULL;
static uintmax_t len = 0;
static uintmax_t cap = 0;

#define grow 32

static void *append(void *thing, int type)
{
	if (len >= cap) {
		cap += grow;
		things = (struct thing *) realloc(things, cap * sizeof (struct thing));
		if (things == NULL)
			die("reallocating things array in test/spaced.c append()");
	}
	things[len].ptr = thing;
	things[len].type = type;
	len++;
	return things[len - 1].ptr;
}

enum types {
	window,
	box,
	tab,
};

void setSpaced(int spaced)
{
	uintmax_t i;
	void *p;

	for (i = 0; i < len; i++) {
		p = things[i].ptr;
		switch (things[i].type) {
		case window:
			uiWindowSetMargined(uiWindow(p), spaced);
			break;
		case box:
			uiBoxSetPadded(uiBox(p), spaced);
			break;
		case tab:
			// TODO
			break;
		}
	}
}

uiWindow *newWindow(const char *title, int width, int height, int hasMenubar)
{
	uiWindow *w;

	w = uiNewWindow(title, width, height, hasMenubar);
	append(w, window);
	return w;
}

uiBox *newHorizontalBox(void)
{
	uiBox *b;

	b = uiNewHorizontalBox();
	append(b, box);
	return b;
}

uiBox *newVerticalBox(void)
{
	uiBox *b;

	b = uiNewVerticalBox();
	append(b, box);
	return b;
}

uiTab *newTab(void)
{
	uiTab *t;

	t = uiNewTab();
	append(t, tab);
	return t;
}
