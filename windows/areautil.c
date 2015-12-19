// 18 december 2015
#include "uipriv_windows.h"
#include "area.h"

// I love COM interfaces that actually only work on C++
// ID2D1RenderTarget::GetSize is defined as returninig a structure
// with stdcall, this means it's an extra last argument
// the compiler tries to return it directly, and crashes
// I originally thought this was a bug in MinGW-w64, but it turns out it also affects MSVC! https://gcc.gnu.org/bugzilla/show_bug.cgi?id=64384
// So we have to work around it.
// TODO is the return type correct? or should we just use C++?
void renderTargetGetSize(ID2D1RenderTarget *rt, D2D1_SIZE_F *size)
{
	typedef void (STDMETHODCALLTYPE *fptr)(ID2D1RenderTarget *, D2D1_SIZE_F *);
	fptr f;

	f = (fptr) (rt->lpVtbl->GetSize);
	(*f)(rt, size);
}
