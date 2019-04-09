// 21 june 2016
#include "test.h"

static uiTableModelHandler mh;

static int modelNumColumns(uiTableModelHandler *mh, uiTableModel *m)
{
	return 9;
}

static uiTableValueType modelColumnType(uiTableModelHandler *mh, uiTableModel *m, int column)
{
	if (column == 3 || column == 4)
		return uiTableValueTypeColor;
	if (column == 5)
		return uiTableValueTypeImage;
	if (column == 7 || column == 8)
		return uiTableValueTypeInt;
	return uiTableValueTypeString;
}

static int modelNumRows(uiTableModelHandler *mh, uiTableModel *m)
{
	return 15;
}

static uiImage *img[2];
static char row9text[1024];
static int yellowRow = -1;
static int checkStates[15];

static uiTableValue *modelCellValue(uiTableModelHandler *mh, uiTableModel *m, int row, int col)
{
	char buf[256];

	if (col == 3) {
		if (row == yellowRow)
			return uiNewTableValueColor(1, 1, 0, 1);
		if (row == 3)
			return uiNewTableValueColor(1, 0, 0, 1);
		if (row == 11)
			return uiNewTableValueColor(0, 0.5, 1, 0.5);
		return NULL;
	}
	if (col == 4) {
		if ((row % 2) == 1)
			return uiNewTableValueColor(0.5, 0, 0.75, 1);
		return NULL;
	}
	if (col == 5) {
		if (row < 8)
			return uiNewTableValueImage(img[0]);
		return uiNewTableValueImage(img[1]);
	}
	if (col == 7)
		return uiNewTableValueInt(checkStates[row]);
	if (col == 8) {
		if (row == 0)
			return uiNewTableValueInt(0);
		if (row == 13)
			return uiNewTableValueInt(100);
		if (row == 14)
			return uiNewTableValueInt(-1);
		return uiNewTableValueInt(50);
	}
	switch (col) {
	case 0:
		sprintf(buf, "Row %d", row);
		break;
	case 2:
		if (row == 9)
			return uiNewTableValueString(row9text);
		// fall through
	case 1:
		strcpy(buf, "Part");
		break;
	case 6:
		strcpy(buf, "Make Yellow");
		break;
	}
	return uiNewTableValueString(buf);
}

static void modelSetCellValue(uiTableModelHandler *mh, uiTableModel *m, int row, int col, const uiTableValue *val)
{
	if (row == 9 && col == 2)
		strcpy(row9text, uiTableValueString(val));
	if (col == 6) {
		int prevYellowRow;

		prevYellowRow = yellowRow;
		yellowRow = row;
		if (prevYellowRow != -1)
			uiTableModelRowChanged(m, prevYellowRow);
		uiTableModelRowChanged(m, yellowRow);
	}
	if (col == 7)
		checkStates[row] = uiTableValueInt(val);
}

static uiTableModel *m;

uiBox *makePage16(void)
{
	uiBox *page16;
	uiTable *t;
	uiTableParams p;
	uiTableTextColumnOptionalParams tp;

	img[0] = uiNewImage(16, 16);
	appendImageNamed(img[0], "andlabs_16x16test_24june2016.png");
	appendImageNamed(img[0], "andlabs_32x32test_24june2016.png");
	img[1] = uiNewImage(16, 16);
	appendImageNamed(img[1], "tango-icon-theme-0.8.90_16x16_x-office-spreadsheet.png");
	appendImageNamed(img[1], "tango-icon-theme-0.8.90_32x32_x-office-spreadsheet.png");

	strcpy(row9text, "Part");

	memset(checkStates, 0, 15 * sizeof (int));

	page16 = newVerticalBox();

	mh.NumColumns = modelNumColumns;
	mh.ColumnType = modelColumnType;
	mh.NumRows = modelNumRows;
	mh.CellValue = modelCellValue;
	mh.SetCellValue = modelSetCellValue;
	m = uiNewTableModel(&mh);

	memset(&p, 0, sizeof (uiTableParams));
	p.Model = m;
	p.RowBackgroundColorModelColumn = 3;
	t = uiNewTable(&p);
	uiBoxAppend(page16, uiControl(t), 1);

	uiTableAppendTextColumn(t, "Column 1",
		0, uiTableModelColumnNeverEditable, NULL);

	memset(&tp, 0, sizeof (uiTableTextColumnOptionalParams));
	tp.ColorModelColumn = 4;
	uiTableAppendImageTextColumn(t, "Column 2",
		5,
		1, uiTableModelColumnNeverEditable, &tp);
	uiTableAppendTextColumn(t, "Editable",
		2, uiTableModelColumnAlwaysEditable, NULL);

	uiTableAppendCheckboxColumn(t, "Checkboxes",
		7, uiTableModelColumnAlwaysEditable);
	uiTableAppendButtonColumn(t, "Buttons",
		6, uiTableModelColumnAlwaysEditable);

	uiTableAppendProgressBarColumn(t, "Progress Bar",
		8);

	return page16;
}

void freePage16(void)
{
	uiFreeTableModel(m);
	uiFreeImage(img[1]);
	uiFreeImage(img[0]);
}
