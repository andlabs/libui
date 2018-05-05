#include "uipriv_windows.hpp"

static void uiTableDestroy(uiControl *c);
static void uiTableMinimumSize(uiWindowsControl *c, int *width, int *height);
static BOOL onWM_NOTIFY(uiControl *c, HWND hwnd, NMHDR *nm, LRESULT *lResult);

struct uiTable;

struct uiTableModel {
	uiTableModelHandler *mh;
	std::vector<uiTable*> tables;
};

struct uiTableColumn {
	uiTable *t;
	WCHAR *name;
	// don't really support parts (but this would part=>column mappings if we did)
	int modelColumn;	// -1 = none
};

struct uiTable {
	uiWindowsControl c;
	uiTableModel *model;
	HWND hwnd;
	std::vector<uiTableColumn*> columns;
};

void *uiTableModelStrdup(const char *str)
{
	return strdup(str);
}

void *uiTableModelGiveColor(double r, double g, double b, double a)
{
	return 0;	// not implemented
}

uiTableModel *uiNewTableModel(uiTableModelHandler *mh)
{
	uiTableModel *m;

	m = new uiTableModel();
	m->mh = mh;
	return m;
}


void uiFreeTableModel(uiTableModel *m)
{
	delete m;
}

void uiTableModelRowInserted(uiTableModel *m, int newIndex)
{
	LVITEM item;

	item.mask = 0;
	item.iItem = newIndex;
	item.iSubItem = 0; //?
	for (auto t : m->tables) {
		ListView_InsertItem( t->hwnd, &item );
	}
}

void uiTableModelRowChanged(uiTableModel *m, int index)
{
	for (auto t : m->tables) {
		ListView_Update( t->hwnd, index );
	}
}

void uiTableModelRowDeleted(uiTableModel *m, int oldIndex)
{
	for (auto t : m->tables) {
		ListView_DeleteItem( t->hwnd, oldIndex );
	}
}

void uiTableColumnAppendTextPart(uiTableColumn *c, int modelColumn, int expand)
{
	uiTable *t = c->t;
	int lvIndex = 0;

	if (c->modelColumn >=0) {
		return; // multiple parts not implemented
	}
	c->modelColumn = modelColumn;

	// work out appropriate listview index for the column
	for (auto candidate : t->columns) {
		if (candidate == c) {
			break;
		}
		if (candidate->modelColumn >= 0) {
			++lvIndex;
		}
	}
	
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;	/* | LVCF_SUBITEM; */
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 120;	// TODO
	lvc.pszText = c->name;
	ListView_InsertColumn(c->t->hwnd, lvIndex, &lvc);
}

void uiTableColumnAppendImagePart(uiTableColumn *c, int modelColumn, int expand)
{
	// not implemented
}

void uiTableColumnAppendButtonPart(uiTableColumn *c, int modelColumn, int expand)
{
	// not implemented
}

void uiTableColumnAppendCheckboxPart(uiTableColumn *c, int modelColumn, int expand)
{
	// not implemented
}

void uiTableColumnAppendProgressBarPart(uiTableColumn *c, int modelColumn, int expand)
{
	// not implemented
}

void uiTableColumnPartSetEditable(uiTableColumn *c, int part, int editable)
{
	// TODO
}

void uiTableColumnPartSetTextColor(uiTableColumn *c, int part, int modelColumn)
{
	// not implemented
}

// uiTable implementation

uiWindowsControlAllDefaultsExceptDestroy(uiTable)

uiTable *uiNewTable(uiTableModel *model)
{
	uiTable *t;
	int winStyle = WS_CHILD | LVS_AUTOARRANGE | LVS_REPORT | LVS_OWNERDATA | LVS_SINGLESEL;

	uiWindowsNewControl(uiTable, t);
	new(&t->columns) std::vector<uiTableColumn*>();		// (initialising in place)
	t->model = model;
	t->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		WC_LISTVIEW,
		L"",
		winStyle,
		hInstance,
		NULL,
		TRUE);
	model->tables.push_back(t);
	uiWindowsRegisterWM_NOTIFYHandler(t->hwnd, onWM_NOTIFY, uiControl(t));
	ListView_SetExtendedListViewStyle(t->hwnd, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
	// TODO: try LVS_EX_AUTOSIZECOLUMNS
	int n = (*(model->mh->NumRows))(model->mh, model);
	ListView_SetItemCountEx(t->hwnd, n, 0);
	return t;
}

uiTableColumn *uiTableAppendColumn(uiTable *t, const char *name)
{
	uiTableColumn *c = uiprivNew(uiTableColumn);
	c->name = toUTF16(name);
	c->t = t;
	c->modelColumn = -1;	// -1 = unassigned
	// we defer the actual ListView_InsertColumn call until a part is added...
	t->columns.push_back(c);
	return c;
}

void uiTableSetRowBackgroundColorModelColumn(uiTable *t, int modelColumn)
{
	// not implemented
}

static void uiTableDestroy(uiControl *c)
{
	uiTable *t = uiTable(c);
	uiTableModel *model = t->model;
	std::vector<uiTable*>::iterator it;

	uiWindowsUnregisterWM_NOTIFYHandler(t->hwnd);
	uiWindowsEnsureDestroyWindow(t->hwnd);
	// detach table from model
	for (it = model->tables.begin(); it != model->tables.end(); ++it) {
		if (*it == t) {
			model->tables.erase(it);
			break;
		}
	}
	// free the columns
	for (auto col: t->columns) {
		uiprivFree(col->name);
		uiprivFree(col);
	}
	t->columns.~vector<uiTableColumn*>();	// (created with placement new, so just call dtor directly)
	uiFreeControl(uiControl(t));
}

static void uiTableMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	uiTable *t = uiTable(c);
	uiWindowsSizing sizing;
	int x, y;

	// suggested listview sizing from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing:
	// "columns widths that avoid truncated data x an integral number of items"
	// Don't think that'll cut it when some cells have overlong data (eg
	// stupidly long URLs). So for now, just hardcode a minimum:

	x = 107;	// in line with other controls
	y = 14*3;	// header + 2 lines (roughly)
	uiWindowsGetSizing(t->hwnd, &sizing);
	uiWindowsSizingDlgUnitsToPixels(&sizing, &x, &y);
	*width = x;
	*height = y;
}

static BOOL onWM_NOTIFY(uiControl *c, HWND hwnd, NMHDR *nm, LRESULT *lResult)
{
	uiTable *t = uiTable(c);
	uiTableModelHandler *mh = t->model->mh;
	BOOL ret = FALSE;

	switch (nm->code) {
	case LVN_GETDISPINFO:
	{
		NMLVDISPINFO* di = (NMLVDISPINFO*)nm;	
		LVITEM* item = &di->item;
		if (!(item->mask & LVIF_TEXT)) {
			break;
		}
		int row = item->iItem;
		int col = item->iSubItem;
		if (col<0 || col>=(int)t->columns.size()) {
			break;
		}

		uiTableColumn *tc = (uiTableColumn*)t->columns[col];

		int mcol = tc->modelColumn;
		uiTableModelColumnType typ = (*mh->ColumnType)(mh,t->model,mcol);
		if (typ == uiTableModelColumnString) {
			void* data = (*(mh->CellValue))(mh, t->model, row, mcol);
			int n = MultiByteToWideChar(CP_UTF8, 0, (const char*)data, -1, item->pszText, item->cchTextMax);
			// make sure clipped strings are nul-terminated
			if (n>=item->cchTextMax) {
				item->pszText[item->cchTextMax-1] = L'\0';
			}
		} else if (typ == uiTableModelColumnInt) {
			char buf[32];
			intptr_t data = (intptr_t)(*(mh->CellValue))(mh, t->model, row, mcol);
			sprintf(buf, "%d", (int)data);
			int n = MultiByteToWideChar(CP_UTF8, 0, buf, -1, item->pszText, item->cchTextMax);
			// make sure clipped strings are nul-terminated
			if (n>=item->cchTextMax) {
				item->pszText[item->cchTextMax-1] = L'\0';
			}
		} else {
			item->pszText[0] = L'\0';
		}
		break;
	}
	default:
		break;
	}
	*lResult = 0;
	return ret;
}

