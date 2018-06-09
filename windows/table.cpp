#include "uipriv_windows.hpp"

struct uiTableModel {
	uiTableModelHandler *mh;
	std::vector<uiTable *> *tables;
};

static uiTableTextColumnOptionalParams defaultTextColumnOptionalParams = {
	/*TODO.ColorModelColumn = */-1,
};

struct columnParams {
	int textModelColumn;
	int textEditableColumn;
	uiTableTextColumnOptionalParams textParams;

	int imageModelColumn;

	int checkboxModelColumn;
	int checkboxEditableColumn;

	int progressBarModelColumn;

	int buttonModelColumn;
	int buttonClickableModelColumn;
};

struct uiTable {
	uiWindowsControl c;
	uiTableModel *model;
	HWND hwnd;
	std::vector<struct columnParams *> *columns;
	WPARAM nColumns;
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
	bool queueUpdated = false;

	wstr = t->dispinfoStrings->front();
	if (wstr != NULL)
		uiprivFree(wstr);
	t->dispinfoStrings->pop();

	p = (*(t->columns))[nm->item.iSubItem];
	// TODO is this condition ever not going to be true for libui?
	if ((nm->item.mask & LVIF_TEXT) != 0)
		if (p->textModelColumn != -1) {
			data = (*(t->model->mh->CellValue))(t->model->mh, t->model, nm->item.iItem, p->textModelColumn);
			wstr = toUTF16(uiTableDataString(data));
			uiFreeTableData(data);
			nm->item.pszText = wstr;
			t->dispinfoStrings->push(wstr);
			queueUpdated = true;
		}

	// we don't want to pop from an empty queue, so if nothing updated the queue (no info was filled in above), just push NULL
	if (!queueUpdated)
		t->dispinfoStrings->push(NULL);
	return 0;
}

static BOOL onWM_NOTIFY(uiControl *c, HWND hwnd, NMHDR *nmhdr, LRESULT *lResult)
{
	uiTable *t = uiTable(c);

	switch (nmhdr->code) {
	case LVN_GETDISPINFO:
		*lResult = onLVN_GETDISPINFO(t, (NMLVDISPINFOW *) nmhdr);
		return TRUE;
	}
	return FALSE;
}

static void uiTableDestroy(uiControl *c)
{
	uiTable *t = uiTable(c);
	uiTableModel *model = t->model;
	std::vector<uiTable *>::iterator it;
	WCHAR *wstr;

	uiWindowsUnregisterWM_NOTIFYHandler(t->hwnd);
	uiWindowsEnsureDestroyWindow(t->hwnd);
	// detach table from model
	for (it = model->tables->begin(); it != model->tables->end(); it++) {
		if (*it == t) {
			model->tables->erase(it);
			break;
		}
	}
	// empty the string queue
	while (t->dispinfoStrings->size() != 0) {
		wstr = t->dispinfoStrings->front();
		if (wstr != NULL)
			uiprivFree(wstr);
		t->dispinfoStrings->pop();
	}
	delete t->dispinfoStrings;
	// free the columns
	for (auto col : *(t->columns))
		uiprivFree(col);
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
#define tableMinHeight (14 * 3)	/* header + 2 lines (roughly) */

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

static struct columnParams *appendColumn(uiTable *t, const char *name, int colfmt)
{
	WCHAR *wstr;
	LVCOLUMNW lvc;
	struct columnParams *p;

	ZeroMemory(&lvc, sizeof (LVCOLUMNW));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt = colfmt;
	lvc.cx = 120;			// TODO
	wstr = toUTF16(name);
	lvc.pszText = wstr;
	if (SendMessageW(t->hwnd, LVM_INSERTCOLUMNW, t->nColumns, (LPARAM) (&lvc)) == (LRESULT) (-1))
		logLastError(L"error calling LVM_INSERTCOLUMNW in appendColumn()");
	uiprivFree(wstr);
	t->nColumns++;

	p = uiprivNew(struct columnParams);
	p->textModelColumn = -1;
	p->imageModelColumn = -1;
	p->checkboxModelColumn = -1;
	p->progressBarModelColumn = -1;
	p->buttonModelColumn = -1;
	t->columns->push_back(p);
	return p;
}

void uiTableAppendTextColumn(uiTable *t, const char *name, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *params)
{
	struct columnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->textModelColumn = textModelColumn;
	p->textEditableColumn = textEditableModelColumn;
	if (params != NULL)
		p->textParams = *params;
	else
		p->textParams = defaultTextColumnOptionalParams;
}

void uiTableAppendImageColumn(uiTable *t, const char *name, int imageModelColumn)
{
	// TODO
}

void uiTableAppendImageTextColumn(uiTable *t, const char *name, int imageModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	struct columnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT | LVCFMT_IMAGE);
	p->textModelColumn = textModelColumn;
	p->textEditableColumn = textEditableModelColumn;
	if (textParams != NULL)
		p->textParams = *textParams;
	else
		p->textParams = defaultTextColumnOptionalParams;
	p->imageModelColumn = imageModelColumn;
}

void uiTableAppendCheckboxColumn(uiTable *t, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn)
{
	// TODO
}

void uiTableAppendCheckboxTextColumn(uiTable *t, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	struct columnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->textModelColumn = textModelColumn;
	p->textEditableColumn = textEditableModelColumn;
	if (textParams != NULL)
		p->textParams = *textParams;
	else
		p->textParams = defaultTextColumnOptionalParams;
	p->checkboxModelColumn = checkboxModelColumn;
	p->checkboxEditableColumn = checkboxEditableModelColumn;
}

void uiTableAppendProgressBarColumn(uiTable *t, const char *name, int progressModelColumn)
{
	// TODO
}

void uiTableAppendButtonColumn(uiTable *t, const char *name, int buttonTextModelColumn, int buttonClickableModelColumn)
{
	// TODO
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

	t->columns = new std::vector<struct columnParams *>;
	t->model = model;
	t->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		WC_LISTVIEW, L"",
		LVS_REPORT | LVS_OWNERDATA | LVS_SINGLESEL | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL,
		hInstance, NULL,
		TRUE);
	model->tables->push_back(t);
	uiWindowsRegisterWM_NOTIFYHandler(t->hwnd, onWM_NOTIFY, uiControl(t));

	// TODO: try LVS_EX_AUTOSIZECOLUMNS
	SendMessageW(t->hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE,
		(WPARAM) (LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP),
		(LPARAM) (LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP));
	n = (*(model->mh->NumRows))(model->mh, model);
	if (SendMessageW(t->hwnd, LVM_SETITEMCOUNT, (WPARAM) n, 0) == 0)
		logLastError(L"error calling LVM_SETITEMCOUNT in uiNewTable()");

	t->dispinfoStrings = new std::queue<WCHAR *>;
	// this encodes the idea that two LVN_GETDISPINFOs must complete before we can free a string: the first real one is for the fourth call to free
	t->dispinfoStrings->push(NULL);
	t->dispinfoStrings->push(NULL);
	t->dispinfoStrings->push(NULL);

	return t;
}
