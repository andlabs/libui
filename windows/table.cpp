#include "uipriv_windows.hpp"

struct uiTableModel {
	uiTableModelHandler *mh;
	std::vector<uiTable *> *tables;
};

struct columnParams {
	int textModelColumn;
	int textEditableColumn;
};

struct uiTable {
	uiWindowsControl c;
	uiTableModel *model;
	HWND hwnd;
	std::vector<struct columnParam *> *columns;
	// MSDN says we have to keep LVN_GETDISPINFO strings we allocate around at least until "two additional LVN_GETDISPINFO messages have been sent".
	// we'll use this queue to do so; the "two additional" part is encoded in the initial state of the queue
	std::queue<WCHAR *> *dispinfoStrings;
};

uiTableModel *uiNewTableModel(uiTableModelHandler *mh)
{
	uiTableModel *m;

	m = uiprivNew(uiTableModel);
	m->mh = mh;
	m->tables = new std::vector<uiTable *>;
	return m;
}

void uiFreeTableModel(uiTableModel *m)
{
	delete m->tables;
	uiprivFree(m);
}

// TODO document that when this is called, the model must return the new row count when asked
void uiTableModelRowInserted(uiTableModel *m, int newIndex)
{
	LVITEMW item;
	int newCount;

	newCount = (*(m->mh->NumRows))(m->mh, m);
	ZeroMemory(&item, sizeof (LVITEMW));
	item.mask = 0;
	item.iItem = newIndex;
	item.iSubItem = 0;
	for (auto t : *(m->tables)) {
		// actually insert the rows
		if (SendMessageW(t->hwnd, LVM_SETITEMCOUNT, (WPARAM) newCount, LVSICF_NOINVALIDATEALL) == 0)
			logLastError(L"error calling LVM_SETITEMCOUNT in uiTableModelRowInserted()");
		// and redraw every row from the new row down to simulate adding it
		if (SendMessageW(t->hwnd, LVM_REDRAWITEMS, (WPARAM) newIndex, (LPARAM) (newCount - 1)) == FALSE)
			logLastError(L"error calling LVM_REDRAWITEMS in uiTableModelRowInserted()");

		// update selection state
		if (SendMessageW(t->hwnd, LVM_INSERTITEM, 0, (LPARAM) (&item)) == (LRESULT) (-1))
			logLastError(L"error calling LVM_INSERTITEM in uiTableModelRowInserted() to update selection state");
	}
}

// TODO compare LVM_UPDATE and LVM_REDRAWITEMS
void uiTableModelRowChanged(uiTableModel *m, int index)
{
	for (auto t : *(m->tables))
		if (SendMessageW(t->hwnd, LVM_UPDATE, (WPARAM) index, 0) == (LRESULT) (-1))
			logLastError(L"error calling LVM_UPDATE in uiTableModelRowChanged()");
}

// TODO document that when this is called, the model must return the OLD row count when asked
// TODO for this and the above, see what GTK+ requires and adjust accordingly
void uiTableModelRowDeleted(uiTableModel *m, int oldIndex)
{
	int newCount;

	newCount = (*(m->mh->NumRows))(m->mh, m);
	newCount--;
	for (auto t : *(m->tables)) {
		// update selection state
		if (SendMessageW(t->hwnd, LVM_DELETEITEM, (WPARAM) oldIndex, 0) == (LRESULT) (-1))
			logLastError(L"error calling LVM_DELETEITEM in uiTableModelRowDeleted() to update selection state");

		// actually delete the rows
		if (SendMessageW(t->hwnd, LVM_SETITEMCOUNT, (WPARAM) newCount, LVSICF_NOINVALIDATEALL) == 0)
			logLastError(L"error calling LVM_SETITEMCOUNT in uiTableModelRowDeleted()");
		// and redraw every row from the new nth row down to simulate removing the old nth row
		if (SendMessageW(t->hwnd, LVM_REDRAWITEMS, (WPARAM) oldIndex, (LPARAM) (newCount - 1)) == FALSE)
			logLastError(L"error calling LVM_REDRAWITEMS in uiTableModelRowDeleted()");
	}
}

static LRESULT onLVN_GETDISPINFO(uiTable *t, NMLVDISPINFOW *nm)
{
	struct columnParams *p;
	uiTableData *data;
	WCHAR *wstr;

	wstr = t->dipsinfoString->front();
	t->dispinfoString->pop();
	uiprivFree(wstr);

	p = (*(t->columns))[nm->item.iSubItem];
	// TODO is this condition ever not going to be true for libui?
	if ((nm->item.mask & LVIF_TEXT) != 0)
		if (p->textModelColumn != -1) {
			data = (*(t->model->mh->Value))(t->model->mh, t->model, nm->item.iItem, p->textModelColumn);
			wstr = toUTF16(uiTableDataString(data));
			uiFreeTableData(data);
			nm->item.pszText = wstr;
			t->dispinfoString->push(wstr);
		}

	return 0;
}

static BOOL onWM_NOTIFY(uiControl *c, HWND hwnd, NMHDR *nmhdr, LRESULT *lResult)
{
	uiTable *t = uiTable(c);
	uiTableModelHandler *mh = t->model->mh;

	switch (nmhdr->code) {
	case LVN_GETDISPINFO:
		*lResult = onLVN_GETDISPINFO(t, (NMLVDISPINFOW *) nmhdr);
		return TRUE;
	}
	return FALSE;
}

$ TODO ===================

static void uiTableDestroy(uiControl *c)
{
	uiTable *t = uiTable(c);
	uiTableModel *model = t->model;
	std::vector<uiTable *>::iterator it;

	uiWindowsUnregisterWM_NOTIFYHandler(t->hwnd);
	uiWindowsEnsureDestroyWindow(t->hwnd);
	// detach table from model
	for (it = model->tables.begin(); it != model->tables.end(); it++) {
		if (*it == t) {
			model->tables.erase(it);
			break;
		}
	}
	// free the columns
	for (auto col : *(t->columns)) {
		uiprivFree(col->name);
		uiprivFree(col);
	}
	delete t->columns;
	uiFreeControl(uiControl(t));
}

uiWindowsControlAllDefaultsExceptDestroy(uiTable)

// suggested listview sizing from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing:
// "columns widths that avoid truncated data x an integral number of items"
// Don't think that'll cut it when some cells have overlong data (eg
// stupidly long URLs). So for now, just hardcode a minimum.
// TODO: Investigate using LVM_GETHEADER/HDM_LAYOUT here...
#define tableMinWidth 107		/* in line with other controls */
#define tableMinHeight (14*3)	/* header + 2 lines (roughly) */

static void uiTableMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	uiTable *t = uiTable(c);
	uiWindowsSizing sizing;
	int x, y;

	x = tableMinWidth;
	y = tableMinHeight;
	uiWindowsGetSizing(t->hwnd, &sizing);
	uiWindowsSizingDlgUnitsToPixels(&sizing, &x, &y);
	*width = x;
	*height = y;
}

void uiTableColumnAppendTextPart(uiTableColumn *c, int modelColumn, int expand)
{
	uiTable *t = c->t;
	int lvIndex = 0;
	LVCOLUMNW lvc;

	if (c->modelColumn >= 0)
		return; // multiple parts not implemented
	c->modelColumn = modelColumn;

	// work out appropriate listview index for the column
	for (auto candidate : *(t->columns)) {
		if (candidate == c)
			break;
		if (candidate->modelColumn >= 0)
			lvIndex++;
	}

	ZeroMemory(&lvc, sizeof (LVCOLUMNW));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;	/* | LVCF_SUBITEM; */
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 120;	// TODO
	lvc.pszText = c->name;
	if (SendMessageW(c->t->hwnd, LVM_INSERTCOLUMN, (WPARAM) lvIndex, (LPARAM) (&lvc)) == (LRESULT) (-1))
		logLastError(L"error calling LVM_INSERTCOLUMN in uiTableColumnPartSetTextPart()");
}

void uiTableColumnPartSetEditable(uiTableColumn *c, int part, int editable)
{
	// TODO
}

uiTableColumn *uiTableAppendColumn(uiTable *t, const char *name)
{
	uiTableColumn *c;
   
	c = uiprivNew(uiTableColumn);
	c->name = toUTF16(name);
	c->t = t;
	c->modelColumn = -1;	// -1 = unassigned
	// we defer the actual ListView_InsertColumn call until a part is added...
	t->columns->push_back(c);
	return c;
}

void uiTableSetRowBackgroundColorModelColumn(uiTable *t, int modelColumn)
{
	// not implemented
}

uiTable *uiNewTable(uiTableModel *model)
{
	uiTable *t;
	int n;

	uiWindowsNewControl(uiTable, t);

	t->columns = new std::vector<uiTableColumn *>;
	t->model = model;
	t->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		WC_LISTVIEW, L"",
		LVS_REPORT | LVS_OWNERDATA | LVS_SINGLESEL | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL,
		hInstance, NULL,
		TRUE);
	model->tables.push_back(t);
	uiWindowsRegisterWM_NOTIFYHandler(t->hwnd, onWM_NOTIFY, uiControl(t));

	// TODO: try LVS_EX_AUTOSIZECOLUMNS
	SendMessageW(t->hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE,
		(WPARAM) (LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP),
		(LPARAM) (LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP));
	n = (*(model->mh->NumRows))(model->mh, model);
	if (SendMessageW(t->hwnd, LVM_SETITEMCOUNT, (WPARAM) n, 0) == 0)
		logLastError(L"error calling LVM_SETITEMCOUNT in uiNewTable()");
	return t;
}

