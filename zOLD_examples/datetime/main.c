#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../ui.h"

uiDateTimePicker *dtboth, *dtdate, *dttime;

const char *timeFormat(uiDateTimePicker *d)
{
	const char *fmt;

	if (d == dtboth)
		fmt = "%c";
	else if (d == dtdate)
		fmt = "%x";
	else if (d == dttime)
		fmt = "%X";
	else
		fmt = "";
	return fmt;
}

void onChanged(uiDateTimePicker *d, void *data)
{
	struct tm time;
	char buf[64];

	uiDateTimePickerTime(d, &time);
	strftime(buf, sizeof (buf), timeFormat(d), &time);
	uiLabelSetText(uiLabel(data), buf);
}

void onClicked(uiButton *b, void *data)
{
	intptr_t now;
	time_t t;
	struct tm tmbuf;

	now = (intptr_t) data;
	t = 0;
	if (now)
		t = time(NULL);
	tmbuf = *localtime(&t);

	if (now) {
		uiDateTimePickerSetTime(dtdate, &tmbuf);
		uiDateTimePickerSetTime(dttime, &tmbuf);
	} else
		uiDateTimePickerSetTime(dtboth, &tmbuf);
}

int onClosing(uiWindow *w, void *data)
{
	uiQuit();
	return 1;
}

int main(void)
{
	uiInitOptions o;
	const char *err;
	uiWindow *w;
	uiGrid *g;
	uiLabel *l;
	uiButton *b;

	memset(&o, 0, sizeof (uiInitOptions));
	err = uiInit(&o);
	if (err != NULL) {
		fprintf(stderr, "error initializing ui: %s\n", err);
		uiFreeInitError(err);
		return 1;
	}

	w = uiNewWindow("Date / Time", 320, 240, 0);
	uiWindowSetMargined(w, 1);

	g = uiNewGrid();
	uiGridSetPadded(g, 1);
	uiWindowSetChild(w, uiControl(g));

	dtboth = uiNewDateTimePicker();
	dtdate = uiNewDatePicker();
	dttime = uiNewTimePicker();

	uiGridAppend(g, uiControl(dtboth),
		0, 0, 2, 1,
		1, uiAlignFill, 0, uiAlignFill);
	uiGridAppend(g, uiControl(dtdate),
		0, 1, 1, 1,
		1, uiAlignFill, 0, uiAlignFill);
	uiGridAppend(g, uiControl(dttime),
		1, 1, 1, 1,
		1, uiAlignFill, 0, uiAlignFill);

	l = uiNewLabel("");
	uiGridAppend(g, uiControl(l),
		0, 2, 2, 1,
		1, uiAlignCenter, 0, uiAlignFill);
	uiDateTimePickerOnChanged(dtboth, onChanged, l);
	l = uiNewLabel("");
	uiGridAppend(g, uiControl(l),
		0, 3, 1, 1,
		1, uiAlignCenter, 0, uiAlignFill);
	uiDateTimePickerOnChanged(dtdate, onChanged, l);
	l = uiNewLabel("");
	uiGridAppend(g, uiControl(l),
		1, 3, 1, 1,
		1, uiAlignCenter, 0, uiAlignFill);
	uiDateTimePickerOnChanged(dttime, onChanged, l);

	b = uiNewButton("Now");
	uiButtonOnClicked(b, onClicked, (void *) 1);
	uiGridAppend(g, uiControl(b),
		0, 4, 1, 1,
		1, uiAlignFill, 1, uiAlignEnd);
	b = uiNewButton("Unix epoch");
	uiButtonOnClicked(b, onClicked, (void *) 0);
	uiGridAppend(g, uiControl(b),
		1, 4, 1, 1,
		1, uiAlignFill, 1, uiAlignEnd);

	uiWindowOnClosing(w, onClosing, NULL);
	uiControlShow(uiControl(w));
	uiMain();
	return 0;
}
