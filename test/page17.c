#include <assert.h>
#include <string.h>

#include "test.h"

struct rgba {
	double r, g, b, a;
};

struct row {
	char *rowxx;
	char *partEditable;
	struct rgba bgColor;
	struct rgba textColor;
	uiImage *image;
	int checkbox;
	int progress;
};

struct data {
	int numRows;
	struct row *rows;
};

enum MCOL {
	MCOL_ROWXX,
	MCOL_PART,
	MCOL_PART_EDITABLE,
	MCOL_BG_COLOR,
	MCOL_TEXT_COLOR,
	MCOL_IMAGE,
	MCOL_BUTTON_TEXT,
	MCOL_CHECKBOX,
	MCOL_PROGRESS,
	MCOL_SIZE
};

#define ROWXX_SIZE 20
#define PART_EDITABLE_SIZE 20

static uiImage *img[2];
static uiEntry *input;
static uiEntry *col1Text;
static uiTableModel *m;
static struct data data;

static int modelNumColumns(uiTableModelHandler *mh, uiTableModel *m)
{
	return MCOL_SIZE;
}

static uiTableValueType modelColumnType(uiTableModelHandler *mh, uiTableModel *m, int column)
{
	switch (column) {
		case MCOL_ROWXX:
		case MCOL_PART:
		case MCOL_PART_EDITABLE:
		case MCOL_BUTTON_TEXT:
			return uiTableValueTypeString;
		case MCOL_BG_COLOR:
		case MCOL_TEXT_COLOR:
			return uiTableValueTypeColor;
		case MCOL_IMAGE:
			return uiTableValueTypeImage;
		case MCOL_CHECKBOX:
		case MCOL_PROGRESS:
			return uiTableValueTypeInt;
		default:
			assert(0);
	}
}

static int modelNumRows(uiTableModelHandler *mh, uiTableModel *m)
{
	return data.numRows;
}

static uiTableValue *modelCellValue(uiTableModelHandler *mh, uiTableModel *m, int row, int column)
{
	switch (column) {
		case MCOL_ROWXX:
			return uiNewTableValueString(data.rows[row].rowxx);
		case MCOL_PART:
			return uiNewTableValueString("Part");
		case MCOL_PART_EDITABLE:
			return uiNewTableValueString(data.rows[row].partEditable);
		case MCOL_BG_COLOR:
			if (data.rows[row].bgColor.a == 0)
				return NULL;
			else
				return uiNewTableValueColor(data.rows[row].bgColor.r, data.rows[row].bgColor.g,
					data.rows[row].bgColor.b, data.rows[row].bgColor.a);
		case MCOL_TEXT_COLOR:
			if (data.rows[row].textColor.a == 0)
				return NULL;
			else
				return uiNewTableValueColor(data.rows[row].textColor.r, data.rows[row].textColor.g,
					data.rows[row].textColor.b, data.rows[row].textColor.a);
		case MCOL_IMAGE:
			return uiNewTableValueImage(data.rows[row].image);
		case MCOL_BUTTON_TEXT:
			return uiNewTableValueString("Make Yellow");
		case MCOL_CHECKBOX:
			return uiNewTableValueInt(data.rows[row].checkbox);
		case MCOL_PROGRESS:
			return uiNewTableValueInt(data.rows[row].progress);
		default:
			assert(0);
	}
}

static void modelSetCellValue(uiTableModelHandler *mh, uiTableModel *m, int row, int column, const uiTableValue *val)
{
	struct rgba yellow = {1, 1, 0, 1};

	switch (column) {
		case MCOL_CHECKBOX:
			data.rows[row].checkbox = uiTableValueInt(val);
			return;
		case MCOL_PART_EDITABLE:
			if (strlen(uiTableValueString(val)) >= PART_EDITABLE_SIZE)
				return;
			strcpy(data.rows[row].partEditable, uiTableValueString(val));
			return;
		case MCOL_BUTTON_TEXT:
			data.rows[row].bgColor = yellow;
			uiTableModelRowChanged(m, row);
			return;
		default:
			assert(0);
	}
}

static void populateRow(int row, const char *text)
{
	struct rgba purple = {.5, 0, .75, 1};
	struct rgba transparent = {0, 0, 0, 0};

	data.rows[row].rowxx = malloc(ROWXX_SIZE * sizeof(*data.rows[row].rowxx));
	assert(data.rows[row].rowxx != NULL);
	strcpy(data.rows[row].rowxx, text);
	data.rows[row].partEditable = malloc(PART_EDITABLE_SIZE * sizeof(*data.rows[row].partEditable));
	assert(data.rows[row].partEditable != NULL);
	strcpy(data.rows[row].partEditable, "Editable");
	data.rows[row].bgColor = transparent;
	data.rows[row].textColor = (row%2 ==1) ? purple : transparent;
	data.rows[row].image = (row/10%2 == 1) ? img[0] : img[1];
	data.rows[row].checkbox = 0;
	data.rows[row].progress = rand() % 101;
}

static void populateData(void)
{
	int row;
	char text[ROWXX_SIZE];

	data.numRows = 10;
	data.rows = malloc(data.numRows * sizeof(*data.rows));
	assert(data.rows != NULL);

	for (row = 0; row < data.numRows; ++row) {
		sprintf(text, "Row %d", row);
		populateRow(row, text);
	}
}


static void insertRow(uiButton *b, void *_data)
{
	char *text = uiEntryText(input);
	int index = atoi(text);
	uiFreeText(text);

	if (index < 0 || index > data.numRows)
		return;

	data.rows = realloc(data.rows, (data.numRows+1) * sizeof(*data.rows));
	assert(data.rows != NULL);

	memmove(&data.rows[index+1], &data.rows[index], (data.numRows - index) * sizeof(*data.rows));

	text = uiEntryText(col1Text);
	populateRow(index, text);
	uiFreeText(text);

	++data.numRows;
	uiTableModelRowInserted(m, index);
}

static void deleteRow(uiButton *b, void *_data)
{
	char *text = uiEntryText(input);
	int index = atoi(text);
	uiFreeText(text);

	if (index < 0 || index >= data.numRows)
		return;

	free(data.rows[index].rowxx);
	free(data.rows[index].partEditable);

	memmove(&data.rows[index], &data.rows[index+1], (data.numRows - index - 1) * sizeof(*data.rows));
	--data.numRows;
	uiTableModelRowDeleted(m, index);

	data.rows = realloc(data.rows, (data.numRows) * sizeof(*data.rows));
	if (data.numRows != 0)
		assert(data.rows != NULL);
}

static uiTableModelHandler mh = {
	.NumColumns = modelNumColumns,
	.ColumnType = modelColumnType,
	.NumRows = modelNumRows,
	.CellValue = modelCellValue,
	.SetCellValue = modelSetCellValue,
};

uiBox *makePage17(void)
{
	uiBox *page17;
	uiBox *controls;
	uiButton *insert, *delete;
	uiTable *t;
	uiTable *t2;
	uiTableParams p = {0};
	uiTableTextColumnOptionalParams tp = {0};

	img[0] = uiNewImage(16, 16);
	appendImageNamed(img[0], "andlabs_16x16test_24june2016.png");
	appendImageNamed(img[0], "andlabs_32x32test_24june2016.png");
	img[1] = uiNewImage(16, 16);
	appendImageNamed(img[1], "tango-icon-theme-0.8.90_16x16_x-office-spreadsheet.png");
	appendImageNamed(img[1], "tango-icon-theme-0.8.90_32x32_x-office-spreadsheet.png");

	page17 = newVerticalBox();

	controls = newHorizontalBox();
	uiBoxAppend(page17, uiControl(controls), 0);
	input = uiNewEntry();
	uiEntrySetText(input, "Index #");
	uiBoxAppend(controls, uiControl(input), 0);
	col1Text = uiNewEntry();
	uiEntrySetText(col1Text, "Column 1 Text");
	uiBoxAppend(controls, uiControl(col1Text), 0);
	insert = uiNewButton("Insert");
	uiButtonOnClicked(insert, insertRow, NULL);
	uiBoxAppend(controls, uiControl(insert), 0);
	delete = uiNewButton("Delete");
	uiButtonOnClicked(delete, deleteRow, NULL);
	uiBoxAppend(controls, uiControl(delete), 0);

	populateData();

	m = uiNewTableModel(&mh);

	p.Model = m;
	p.RowBackgroundColorModelColumn = 3;
	t = uiNewTable(&p);
	uiBoxAppend(page17, uiControl(t), 1);

	uiTableAppendTextColumn(t, "Column 1",
		0, uiTableModelColumnNeverEditable, NULL);

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

	t2 = uiNewTable(&p);
	uiBoxAppend(page17, uiControl(t2), 1);
	uiTableAppendTextColumn(t2, "Column 1",
		0, uiTableModelColumnNeverEditable, NULL);
	uiTableAppendImageTextColumn(t2, "Column 2",
		5,
		1, uiTableModelColumnNeverEditable, &tp);
	uiTableAppendTextColumn(t2, "Editable",
		2, uiTableModelColumnAlwaysEditable, NULL);
	uiTableAppendCheckboxColumn(t2, "Checkboxes",
		7, uiTableModelColumnAlwaysEditable);

	return page17;
}

void freePage17(void)
{
	int row;
	for (row = 0; row < data.numRows; ++row) {
		free(data.rows[row].rowxx);
		free(data.rows[row].partEditable);
	}
	free(data.rows);

	uiFreeTableModel(m);
	uiFreeImage(img[1]);
	uiFreeImage(img[0]);
}
