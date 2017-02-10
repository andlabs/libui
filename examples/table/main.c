// 2 september 2015
#include <stdio.h>
#include <string.h>
#include "../../ui.h"
//#include "../../uitable.h"

static int onClosing(uiWindow *w, void *data)
{
	uiQuit();
	return 1;
}

static int onShouldQuit(void *data)
{
	uiWindow *mainwin = uiWindow(data);

	uiControlDestroy(uiControl(mainwin));
	return 1;
}


static uiTableModel* model=0;

int handlerNumColumns(uiTableModelHandler *h, uiTableModel *m)
{
    printf("handlerNumColumns()\n");
    return 2;
}

uiTableModelColumnType handlerColumnType(uiTableModelHandler *h, uiTableModel *m, int col)
{
    printf("handlerColumnType(col=%d)\n",col);
    return uiTableModelColumnString;
}


int handlerNumRows(uiTableModelHandler *h, uiTableModel *m)
{
    printf("handlerNumRows()\n");
    return 5;
}

void* handlerCellValue(uiTableModelHandler *h, uiTableModel *m, int row, int col)
{
    static char buf[32];
    printf("handlerCellValue(row %d, col %d)\n", row,col);
    sprintf(buf,"row %d, col %d", row,col);
    return uiTableModelStrdup(buf);
}

void handlerSetCellValue(uiTableModelHandler *h, uiTableModel *m, int row, int col, const void *val)
{
    printf("handlerSetCell(row %d, col %d)\n", row,col);
}

static uiTableModelHandler handler = {
	handlerNumColumns,
    handlerColumnType,
	handlerNumRows,
	handlerCellValue,
	handlerSetCellValue
};

static uiControl *makeTable(void)
{
    model = uiNewTableModel(&handler);
    uiTable* t = uiNewTable(model);
	uiTableAppendTextColumn(t, "Column 1", 0);
	uiTableAppendTextColumn(t, "Column 2", 1);
    return uiControl(t);
}


static uiWindow *mainwin;

int main(void)
{
	uiInitOptions options;
	const char *err;

	memset(&options, 0, sizeof (uiInitOptions));
	err = uiInit(&options);
	if (err != NULL) {
		fprintf(stderr, "error initializing libui: %s", err);
		uiFreeInitError(err);
		return 1;
	}

	mainwin = uiNewWindow("libui Control Gallery", 640, 480, 1);
	uiWindowOnClosing(mainwin, onClosing, NULL);
	uiOnShouldQuit(onShouldQuit, mainwin);

	uiBox *vbox;
	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);
    uiControl* t = makeTable();
	uiBoxAppend(vbox, uiControl(t), 1);

	uiWindowSetChild(mainwin, uiControl(vbox));
	uiWindowSetMargined(mainwin, 1);

	uiControlShow(uiControl(mainwin));
	uiMain();
	return 0;
}

