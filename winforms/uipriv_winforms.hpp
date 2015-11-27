// 25 november 2015
#include <vcclr.h>
#include "../ui.h"
#include "ui_winforms.hpp"
#include "../common/uipriv.h"
#include "unmanaged.h"

#using <System.dll>
#using <System.Windows.Forms.dll>
using namespace System;
using namespace System::Windows::Forms;

// text.cpp
extern String ^fromUTF8(const char *);

// control.cpp
extern Control ^genericHandle(uiControl *c);

typedef gcroot<Label ^> *DUMMY;
static inline DUMMY mkdummy(String ^classname)
{
	DUMMY t;

	t = new gcroot<Label ^>();
	*t = gcnew Label();
	(*t)->Content = L"TODO " + classname + L" not implemented";
	return t;
}
