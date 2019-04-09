// 23 june 2018
#include "../ui.h"
#include "uipriv.h"
#include "table.h"

int uiprivTableModelNumColumns(uiTableModel *m)
{
	uiTableModelHandler *mh;

	mh = uiprivTableModelHandler(m);
	return (*(mh->NumColumns))(mh, m);
}

uiTableValueType uiprivTableModelColumnType(uiTableModel *m, int column)
{
	uiTableModelHandler *mh;

	mh = uiprivTableModelHandler(m);
	return (*(mh->ColumnType))(mh, m, column);
}

int uiprivTableModelNumRows(uiTableModel *m)
{
	uiTableModelHandler *mh;

	mh = uiprivTableModelHandler(m);
	return (*(mh->NumRows))(mh, m);
}

uiTableValue *uiprivTableModelCellValue(uiTableModel *m, int row, int column)
{
	uiTableModelHandler *mh;

	mh = uiprivTableModelHandler(m);
	return (*(mh->CellValue))(mh, m, row, column);
}

void uiprivTableModelSetCellValue(uiTableModel *m, int row, int column, const uiTableValue *value)
{
	uiTableModelHandler *mh;

	mh = uiprivTableModelHandler(m);
	(*(mh->SetCellValue))(mh, m, row, column, value);
}

const uiTableTextColumnOptionalParams uiprivDefaultTextColumnOptionalParams = {
	.ColorModelColumn = -1,
};

int uiprivTableModelCellEditable(uiTableModel *m, int row, int column)
{
	uiTableValue *value;
	int editable;

	switch (column) {
	case uiTableModelColumnNeverEditable:
		return 0;
	case uiTableModelColumnAlwaysEditable:
		return 1;
	}
	value = uiprivTableModelCellValue(m, row, column);
	editable = uiTableValueInt(value);
	uiFreeTableValue(value);
	return editable;
}

int uiprivTableModelColorIfProvided(uiTableModel *m, int row, int column, double *r, double *g, double *b, double *a)
{
	uiTableValue *value;

	if (column == -1)
		return 0;
	value = uiprivTableModelCellValue(m, row, column);
	if (value == NULL)
		return 0;
	uiTableValueColor(value, r, g, b, a);
	uiFreeTableValue(value);
	return 1;
}
