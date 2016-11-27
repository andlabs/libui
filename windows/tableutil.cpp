// 1 july 2016
// TODO includes

void tableGetClientRect(HWND hwnd, RECT *r)
{
	if (GetClientRect(hwnd, r) == 0) {
		r->left = 0;
		r->top = 0;
		r->right = 0;
		r->bottom = 0;
	}
}

void tableGetWindowRect(HWND hwnd, RECT *r)
{
	if (GetWindowRect(hwnd, r) == 0) {
		r->left = 0;
		r->top = 0;
		r->right = 0;
		r->bottom = 0;
	}
}

void tableGetTextExtentPoint32W(HDC dc, const WSTR *str, int len, SIZE *s)
{
	if (len == -1)
		len = wcslen(str);
	if (GetTextExtentPoint32W(dc, str, len, s) == 0) {
		s->cx = 0;
		s->cy = 0;
	}
}
