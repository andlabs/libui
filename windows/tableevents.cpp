// 17 june 2018
#include "uipriv_windows.hpp"
#include "table.hpp"

HRESULT uiprivTableHandleNM_CLICK(uiTable *t, NMITEMACTIVATE *nm, LRESULT *lResult)
{
	LVHITTESTINFO ht;
	uiprivTableColumnParams *p;
	int modelColumn, editableColumn;
	bool text, checkbox;
	uiTableData *data;
	int checked, editable;

	ZeroMemory(&ht, sizeof (LVHITTESTINFO));
	ht.pt = nm->ptAction;
	if (SendMessageW(t->hwnd, LVM_SUBITEMHITTEST, 0, (LPARAM) (&ht)) == (LRESULT) (-1))
		goto done;

	modelColumn = -1;
	editableColumn = -1;
	text = false;
	checkbox = false;
	p = (*(t->columns))[ht.iSubItem];
	if (p->textModelColumn != -1) {
		modelColumn = p->textModelColumn;
		editableColumn = p->textEditableColumn;
		text = true;
	} else if (p->checkboxModelColumn != -1) {
		modelColumn = p->checkboxModelColumn;
		editableColumn = p->checkboxEditableColumn;
		checkbox = true;
	} else if (p->buttonModelColumn != -1) {
		modelColumn = p->buttonModelColumn;
		editableColumn = p->buttonClickableModelColumn;
	}
	if (modelColumn == -1)
		goto done;

	if (text && t->inDoubleClickTimer)
		// don't even ask for info if it's too soon to edit text
		goto done;

	switch (editableColumn) {
	case uiTableModelColumnNeverEditable:
		goto done;
	case uiTableModelColumnAlwaysEditable:
		break;
	default:
		data = (*(t->model->mh->CellValue))(t->model->mh, t->model, ht.iItem, editableColumn);
		editable = uiTableDataInt(data);
		uiFreeTableData(data);
		if (!editable)
			goto done;
	}

	if (text) {
		MessageBoxW(NULL, L"editing text", L"ok", MB_OK);
	} else if (checkbox) {
		if ((ht.flags & LVHT_ONITEMICON) == 0)
			goto done;
		data = (*(t->model->mh->CellValue))(t->model->mh, t->model, ht.iItem, modelColumn);
		checked = uiTableDataInt(data);
		uiFreeTableData(data);
		data = uiNewTableDataInt(!checked);
		(*(t->model->mh->SetCellValue))(t->model->mh, t->model, ht.iItem, modelColumn, data);
		uiFreeTableData(data);
	} else
		(*(t->model->mh->SetCellValue))(t->model->mh, t->model, ht.iItem, modelColumn, NULL);
	// always refresh the value in case the model rejected it
	if (SendMessageW(t->hwnd, LVM_UPDATE, (WPARAM) (ht.iItem), 0) == (LRESULT) (-1)) {
		logLastError(L"LVM_UPDATE");
		return E_FAIL;
	}

done:
	*lResult = 0;
	return S_OK;
}
