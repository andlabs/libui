// 25 november 2015
#include "uipriv_winforms.hpp"
#include <string.h>

using namespace System::Text;
using namespace System::Runtime::InteropServices;

// TODO export?
String ^fromUTF8(const char *str)
{
	array<Byte> ^bytes;
	size_t len;

	len = strlen(str);
	bytes = gcnew array<Byte>(len);
	// TODO avoid the cast
	Marshal::Copy(IntPtr((char *) str), bytes, 0, len);
	return Encoding::UTF8->GetString(bytes);
}

// see http://stackoverflow.com/questions/27526093/convert-systemstring-to-stdstring-in-utf8-which-later-converted-to-char-as
char *uiWindowsCLRStringToText(gcroot<System::String ^> str)
{
	array<Byte> ^bytes;
	char *cstr;

	bytes = Encoding::UTF8->GetBytes(str);
	cstr = (char *) uiAlloc((bytes->Length + 1) * sizeof (char), "char[]");
	Marshal::Copy(bytes, 0, IntPtr(cstr), bytes->Length);
	cstr[bytes->Length] = '\0';
	return cstr;
}

void uiFreeText(char *c)
{
	uiFree(c);
}
