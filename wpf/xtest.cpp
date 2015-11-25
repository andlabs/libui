// 24 november 2015
#using <System.dll>
#using <WindowsBase.dll>
#using <PresentationCore.dll>
#using <PresentationFramework.dll>
using namespace System;
using namespace System::ComponentModel;

int main(void)
{
	System::ComponentModel::CancelEventArgs ^e;

	e = gcnew CancelEventArgs();
	e->Cancel = true;
	System::Console::WriteLine(e->Cancel);
	return 0;
}
