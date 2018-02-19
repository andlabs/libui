struct uiWindow {
//	BOOL hasMenubar;
	void (*onPositionChanged)(uiWindow *, void *);
	void *onPositionChangedData;
	BOOL changingPosition;		// to avoid triggering the above when programmatically doing this
//	void (*onContentSizeChanged)(uiWindow *, void *);
};

static LRESULT CALLBACK windowWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	case WM_WINDOWPOSCHANGED:
		if ((wp->flags & SWP_NOMOVE) == 0)
			if (!w->changingPosition)
				(*(w->onPositionChanged))(w, w->onPositionChangedData);
			// and continue anyway
//		if ((wp->flags & SWP_NOSIZE) != 0)
}

// static int defaultOnClosing(uiWindow *w, void *data)

static void defaultOnPositionContentSizeChanged(uiWindow *w, void *data)
{
	// do nothing
}

// static std::map<uiWindow *, bool> windows;

// void uiWindowSetTitle(uiWindow *w, const char *title)

void uiWindowPosition(uiWindow *w, int *x, int *y)
{
	RECT r;

	uiWindowsEnsureGetWindowRect(w->hwnd, &r);
	*x = r.left;
	*y = r.top;
}

void uiWindowSetPosition(uiWindow *w, int x, int y)
{
	w->changingPosition = TRUE;
	if (SetWindowPos(w->hwnd, NULL, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER) == 0)
		logLastError(L"error moving window");
	w->changingPosition = FALSE;
}

// static void windowMonitorRect(HWND hwnd, RECT *r)

// TODO use the work rect instead?
void uiWindowCenter(uiWindow *w)
{
	RECT wr, mr;
	int x, y;
	LONG wwid, mwid;
	LONG wht, mht;

	uiWindowsEnsureGetWindowRect(w->hwnd, &wr);
	windowMonitorRect(w->hwnd, &mr);
	wwid = wr.right - wr.left;
	mwid = mr.right - mr.left;
	x = (mwid - wwid) / 2;
	wht = wr.bottom - wr.top;
	mht = mr.bottom - mr.top;
	y = (mht - wht) / 2;
	// y is now evenly divided, however https://msdn.microsoft.com/en-us/library/windows/desktop/dn742502(v=vs.85).aspx says that 45% should go above and 55% should go below
	// so just move 5% of the way up
	// TODO should this be on the work area?
	// TODO is this calculation correct?
	y -= y / 20;
	uiWindowSetPosition(w, x, y);
}

void uiWindowOnPositionChanged(uiWindow *w, void (*f)(uiWindow *, void *), void *data)
{
	w->onPositionChanged = f;
	w->onPositionChangedData = data;
}

// void uiWindowContentSize(uiWindow *w, int *width, int *height)

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
//	uiWindowOnClosing(w, defaultOnClosing, NULL);
	uiWindowOnPositionChanged(w, defaultOnPositionContentSizeChanged, NULL);
//	uiWindowOnContentSizeChanged(w, defaultOnPositionContentSizeChanged, NULL);
}
