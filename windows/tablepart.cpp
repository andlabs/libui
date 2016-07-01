// 30 june 2016
// TODO includes

typedef struct tablePartDrawParams tablePartDrawParams;
typedef struct tablePartMinimumSizeParams tableDrawMinimumSizeParams;
typedef struct tablePartEditingParams tablePartEditingParams;

struct tablePartDrawParams {
	HWND hwnd;
	HDC hdc;
	RECT *r;
	bool selected;
	bool focused;
	bool hovering;
	uiTableModel *model;
	int row;
};

struct tablePartMinimumSizeParams {
	HWND hwnd;
	HDC hdc;
	uiTableModel *model;
	int row;
};

enum {
	partEventDoNothing,
	partEventRedraw,
	partEventEdit,
};

struct tablePartEditingParams {
	HWND newHWND;
};

enum {
	partEditContinue,
	partEditDone,
};

class tablePart {
public:
	// needed so we can delete a tablePart
	virtual ~tablePart() {}

	virtual HRESULT Draw(tablePartDrawParams *p) = 0;
	virtual HRESULT MinimumSize(tablePartMinimumSizeParams *p, int *width, int *height) = 0;

	// returns a partEvent constant
	virtual int MouseMove(int x, int y, RECT *cell) = 0;
	virtual int MouseLeave(void) = 0;
	virtual int LButtonDown(int x, int y, int count, RECT *cell) = 0;
	virtual int LButtonUp(int x, int y, RECT *cell) = 0;
	virtual int CaptureBroken(void) = 0;
	virtual int KeyDown(void) = 0;
	virtual int KeyUp(void) = 0;

	// editing; all optional
	virtual int StartEditing(tablePartEditingParams *p) { return editDone; }
	virtual int EditChildWM_COMMAND(WORD code, LRESULT *lr) { return editDone; }
	virtual void FinishEditing(uiTableModel *model, int row) {}
	virtual void CancelEditing(void) {}

	// TODO tooltips
	// TODO timers and animations

	// optional methods
	virtual void SetTextColorColumn(int col) {}
	virtual void SetEditable(bool editable) {}
};

class tablePartText : public tablePart {
	int textColumn;
	int colorColumn;
public:
	tablePartText(int tc)
	{
		this->textColumn = tc;
		this->colorColumn = -1;
	}

	// TODO figure out vertical alignment
	virtual HRESULT Draw(tablePartDrawParams *p)
	{
	}

	virtual HRESULT MinimumSize(tablePartMinimumSizeParams *p, int *width, int *height)
	{
	}
};

tablePart *newTablePartText(int tc)
{
	return new tablePartText(tc);
}
