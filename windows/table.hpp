// 10 june 2018

// table.cpp
#define uiprivNumLVN_GETDISPINFOSkip 3
struct uiTableModel {
	uiTableModelHandler *mh;
	std::vector<uiTable *> *tables;
};
typedef struct uiprivTableColumnParams uiprivTableColumnParams;
struct uiprivTableColumnParams {
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
	std::vector<uiprivTableColumnParams *> *columns;
	WPARAM nColumns;
	int backgroundColumn;

	// owner data state
	// MSDN says we have to keep LVN_GETDISPINFO strings we allocate around at least until "two additional LVN_GETDISPINFO messages have been sent".
	// we'll use this queue to do so; the "two additional" part is encoded in the initial state of the queue
	std::queue<WCHAR *> *dispinfoStrings;

	// tableimages.cpp
	// TODO make sure what we're doing is even allowed
	HIMAGELIST smallImages;
	int smallIndex;

	// custom draw state
	COLORREF clrItemText;
};

// tableimages.cpp
extern HRESULT uiprivLVN_GETDISPINFOImagesCheckboxes(uiTable *t, NMLVDISPINFOW *nm, uiprivTableColumnParams *p);
extern HRESULT uiprivNM_CUSTOMDRAWImagesCheckboxes(uiTable *t, NMLVCUSTOMDRAW *nm, LRESULT *lResult);
extern HRESULT uiprivTableSetupImagesCheckboxes(uiTable *t);
