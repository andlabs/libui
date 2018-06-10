#include "uipriv_windows.hpp"
#include "table.hpp"

static uiTableTextColumnOptionalParams defaultTextColumnOptionalParams = {
	/*TODO.ColorModelColumn = */-1,
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
	static uiprivTableColumnParams *p;
	uiTableData *data;
	WCHAR *wstr;
	HDC dc;
	IWICBitmap *wb;
	HBITMAP b;
	int checked;
	bool queueUpdated = false;
	HRESULT hr;

	wstr = t->dispinfoStrings->front();
	if (wstr != NULL)
		uiprivFree(wstr);
	t->dispinfoStrings->pop();

	p = (*(t->columns))[nm->item.iSubItem];
nm->item.pszText=L"abcdefg";
	if ((nm->item.mask & LVIF_TEXT) != 0)
		if (p->textModelColumn != -1) {
			data = (*(t->model->mh->CellValue))(t->model->mh, t->model, nm->item.iItem, p->textModelColumn);
			wstr = toUTF16(uiTableDataString(data));
			uiFreeTableData(data);
			nm->item.pszText = wstr;
			t->dispinfoStrings->push(wstr);
			queueUpdated = true;
		}

	hr = uiprivLVN_GETDISPINFOImagesCheckboxes(t, nm, p);
	if (hr != S_OK) {
		// TODO
	}

	// we don't want to pop from an empty queue, so if nothing updated the queue (no info was filled in above), just push NULL
	if (!queueUpdated)
		t->dispinfoStrings->push(NULL);
	return 0;
}

static COLORREF blend(COLORREF base, double r, double g, double b, double a)
{
	double br, bg, bb;

	// TODO find a better fix than this
	// TODO s listview already alphablending?
	// TODO find the right color here
	if (base == CLR_DEFAULT)
		base = GetSysColor(COLOR_WINDOW);
	br = ((double) GetRValue(base)) / 255.0;
	bg = ((double) GetGValue(base)) / 255.0;
	bb = ((double) GetBValue(base)) / 255.0;

	br = (r * a) + (br * (1.0 - a));
	bg = (g * a) + (bg * (1.0 - a));
	bb = (b * a) + (bb * (1.0 - a));
	return RGB((BYTE) (br * 255),
		(BYTE) (bg * 255),
		(BYTE) (bb * 255));
}

static LRESULT onNM_CUSTOMDRAW(uiTable *t, NMLVCUSTOMDRAW *nm)
{
	uiprivTableColumnParams *p;
	uiTableData *data;
	double r, g, b, a;
	LRESULT ret;
	HRESULT hr;

	switch (nm->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		ret = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		if (t->backgroundColumn != -1) {
			data = (*(t->model->mh->CellValue))(t->model->mh, t->model, nm->nmcd.dwItemSpec, t->backgroundColumn);
			if (data != NULL) {
				uiTableDataColor(data, &r, &g, &b, &a);
				uiFreeTableData(data);
				nm->clrTextBk = blend(nm->clrTextBk, r, g, b, a);
			}
		}
		t->clrItemText = nm->clrText;
		ret = CDRF_NEWFONT | CDRF_NOTIFYSUBITEMDRAW;
		break;
	case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		p = (*(t->columns))[nm->iSubItem];
		// TODO none of this runs on the first item
		// we need this as previous subitems will persist their colors
		nm->clrText = t->clrItemText;
		if (p->textParams.ColorModelColumn != -1) {
			data = (*(t->model->mh->CellValue))(t->model->mh, t->model, nm->nmcd.dwItemSpec, p->textParams.ColorModelColumn);
			if (data != NULL) {
				uiTableDataColor(data, &r, &g, &b, &a);
				uiFreeTableData(data);
				nm->clrText = blend(nm->clrTextBk, r, g, b, a);
			}
		}
if (nm->iSubItem != 0) {
HWND header;
LRESULT margin;
header = (HWND) SendMessageW(t->hwnd, LVM_GETHEADER, NULL, NULL);
margin = SendMessageW(header, HDM_GETBITMAPMARGIN, 0, 0);
//nm->rcText.left -= margin;
printf("%d %d %d %d\n",
(int)(nm->nmcd.rc.left),
(int)(nm->nmcd.rc.top),
(int)(nm->nmcd.rc.right),
(int)(nm->nmcd.rc.bottom));
FillRect(nm->nmcd.hdc, &(nm->nmcd.rc), GetSysColorBrush(COLOR_ACTIVECAPTION));
return CDRF_SKIPDEFAULT;
}
		// TODO draw background on image columns if needed
		ret = CDRF_NEWFONT;
		break;
	default:
		ret = CDRF_DODEFAULT;
	}

	hr = uiprivNM_CUSTOMDRAWImagesCheckboxes(t, nm, &ret);
	if (hr != S_OK) {
		// TODO
	}
	return ret;
}

static BOOL onWM_NOTIFY(uiControl *c, HWND hwnd, NMHDR *nmhdr, LRESULT *lResult)
{
	uiTable *t = uiTable(c);

	switch (nmhdr->code) {
	case LVN_GETDISPINFO:
		*lResult = onLVN_GETDISPINFO(t, (NMLVDISPINFOW *) nmhdr);
		return TRUE;
	case NM_CUSTOMDRAW:
		*lResult = onNM_CUSTOMDRAW(t, (NMLVCUSTOMDRAW *) nmhdr);
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
	// t->smallImages will be automatically destroyed
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

static uiprivTableColumnParams *appendColumn(uiTable *t, const char *name, int colfmt)
{
	WCHAR *wstr;
	LVCOLUMNW lvc;
	uiprivTableColumnParams *p;

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

	p = uiprivNew(uiprivTableColumnParams);
	p->textModelColumn = -1;
	p->textEditableColumn = -1;
	p->textParams = defaultTextColumnOptionalParams;
	p->imageModelColumn = -1;
	p->checkboxModelColumn = -1;
	p->checkboxEditableColumn = -1;
	p->progressBarModelColumn = -1;
	p->buttonModelColumn = -1;
	t->columns->push_back(p);
	return p;
}

void uiTableAppendTextColumn(uiTable *t, const char *name, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *params)
{
	uiprivTableColumnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->textModelColumn = textModelColumn;
	p->textEditableColumn = textEditableModelColumn;
	if (params != NULL)
		p->textParams = *params;
}

void uiTableAppendImageColumn(uiTable *t, const char *name, int imageModelColumn)
{
	// TODO
}

void uiTableAppendImageTextColumn(uiTable *t, const char *name, int imageModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	uiprivTableColumnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->textModelColumn = textModelColumn;
	p->textEditableColumn = textEditableModelColumn;
	if (textParams != NULL)
		p->textParams = *textParams;
	p->imageModelColumn = imageModelColumn;
}

void uiTableAppendCheckboxColumn(uiTable *t, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn)
{
	uiprivTableColumnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->checkboxModelColumn = checkboxModelColumn;
	p->checkboxEditableColumn = checkboxEditableModelColumn;
}

void uiTableAppendCheckboxTextColumn(uiTable *t, const char *name, int checkboxModelColumn, int checkboxEditableModelColumn, int textModelColumn, int textEditableModelColumn, uiTableTextColumnOptionalParams *textParams)
{
	uiprivTableColumnParams *p;

	p = appendColumn(t, name, LVCFMT_LEFT);
	p->textModelColumn = textModelColumn;
	p->textEditableColumn = textEditableModelColumn;
	if (textParams != NULL)
		p->textParams = *textParams;
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
	// TODO make the names consistent
	t->backgroundColumn = modelColumn;
	// TODO redraw?
}

uiTable *uiNewTable(uiTableModel *model)
{
	uiTable *t;
	int n;
	int i;
	HRESULT hr;

	uiWindowsNewControl(uiTable, t);

	t->columns = new std::vector<uiprivTableColumnParams *>;
	t->model = model;
	t->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		WC_LISTVIEW, L"",
		LVS_REPORT | LVS_OWNERDATA | LVS_SINGLESEL | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL,
		hInstance, NULL,
		TRUE);
	model->tables->push_back(t);
	uiWindowsRegisterWM_NOTIFYHandler(t->hwnd, onWM_NOTIFY, uiControl(t));

	// TODO: try LVS_EX_AUTOSIZECOLUMNS
	// TODO check error
	SendMessageW(t->hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE,
		(WPARAM) (LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_SUBITEMIMAGES),
		(LPARAM) (LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_SUBITEMIMAGES));
	n = (*(model->mh->NumRows))(model->mh, model);
	if (SendMessageW(t->hwnd, LVM_SETITEMCOUNT, (WPARAM) n, 0) == 0)
		logLastError(L"error calling LVM_SETITEMCOUNT in uiNewTable()");

	t->backgroundColumn = -1;

	t->dispinfoStrings = new std::queue<WCHAR *>;
	// this encodes the idea that two LVN_GETDISPINFOs must complete before we can free a string: the first real one is for the fourth call to free
	for (i = 0; i < uiprivNumLVN_GETDISPINFOSkip; i++)
		t->dispinfoStrings->push(NULL);

	hr = uiprivTableSetupImagesCheckboxes(t);
	if (hr != S_OK) {
		// TODO
	}

	return t;
}
