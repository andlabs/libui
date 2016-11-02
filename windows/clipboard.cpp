#include "uipriv_windows.hpp"

int uiClipboardSetText(const char *text)
{
	if (!OpenClipboard(NULL)) {
		return FALSE;
	}	
	if (!EmptyClipboard()) {
		CloseClipboard();
		return FALSE;
	}

	int result = TRUE;
	WCHAR *wtext = toUTF16(text);
	size_t size = wcslen(wtext) * sizeof(WCHAR) + sizeof(WCHAR);
	HGLOBAL handle = GlobalAlloc(NULL, size);

	if (handle) {
		WCHAR *dst = (WCHAR *)GlobalLock(handle);
		if (dst) {
			memcpy(dst, wtext, size);
			GlobalUnlock(handle);
			if (!SetClipboardData(CF_UNICODETEXT, handle)) {
				result = FALSE;
			}
		}
		else {
			GlobalFree(handle);
			result = FALSE;
		}
	}
	
	uiFree(wtext);
	CloseClipboard();
	return result;
}

char *uiClipboardGetText()
{
	if (!OpenClipboard(NULL)) {
		return NULL;
	}

	char *result = NULL;
	HANDLE handle = GetClipboardData(CF_UNICODETEXT);

	if (handle) {
		WCHAR *data = (WCHAR *)GlobalLock(handle);
		if (data) {
			result = toUTF8(data);
			GlobalUnlock(data);
		}
	}

	CloseClipboard();
	return result;
}