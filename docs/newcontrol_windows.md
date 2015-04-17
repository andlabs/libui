# Writing new controls for the Windows backend

The Unix backend uses the raw Windows API to do its UI work. The good news is that you don't need to implement most of uiControl to make things work. The bad news is that you *do* need to implement uiControlPreferredSize(). (TODO rewrite this?)

For this document, we will write a basic wrapper around the [trackbar control](xxxTODOxxxx) as a slider that goes from 0 to 99 inclusive. It will conform to the following interface:

```c
typedef struct mySlider mySlider;
struct mySlider {
	uiControl base;
	int (*Pos)(mySlider *);
};
#define mySlider(x) ((mySwitch *) (x))
#define mySliderPos(s) ((*((s)->Pos))((s)))
```

Error checking will be omitted for the purposes of demonstration.

To be able to create Windows backend controls, we need to include `ui_windows.h`, the file that contains the Windows backend function declarations. It requires that you include both `ui.h` and `<windows.h>` (and any other macros like `UNICODE` and Windows API headers like `<commctrl.h>`) beforehand:

```c
#define UNICODE
#define _UNICODE
#define STRICT
#include <windows.h>
#include <commctrl.h>
#include "ui.h"
#include "ui_windows.h"
```

Before we can create trackbars, though, we need to make sure the trackbar control class is loaded. As the trackbar is provided by the Common Controls library, we do this with the `InitCommonControlsEx()` function:

```c
static void registerSliderClass(void)
{
	static int initialized = 0;
	INITCOMMONCONTROLSEX icc;

	if (initialized)
		return;
	initialized = 1;
	ZeroMemory(&icc, sizeof (INITCOMMONCONTROLSEX));
	icc.dwSize = sizeof (INITCOMMONCONTROLSEX);
	icc.dwICC = xxxTODOxxxxxx;
	InitCommonControlsEx(&icc);
}
```

Now that we have that done, we can move on to writing the control itself. The first thing we need to do is define the data structure used for our new control. The first field of this data structure must be a mySlider (not a pointer to a mySlider). This allows us to use our data structure *as* the mySlider:

```c
struct slider {
	mySlider s;
```

We'll also store a copy of the slider's HWND to make things easier for us:

```c
	HWND hwnd;
};
```

The good news is that most of the work of implementing uiControl is done for you by libui. You only need to provide a few things:

- the parameters to the `CreateWindowExW()` call that creates the control's window
- handlers for `WM_COMMAND` and `WM_NOTIFY` that handle any `WM_COMMAND` and `WM_NOTIFY` messages the control sends to its parent window
- a handler for `WM_DESTROY` that does any cleanup that needs to be done
- an implementation of `uiControlPreferredSize()`
- the other methods of your interface

We'll deal with the first bullet later.

The handlers for `WM_COMMAND` and `WM_NOTIFY` that libui expects to see have specific signatures:

```c
BOOL onWM_COMMAND(uiControl *c, WORD code, LRESULT *lResult);
BOOL onWM_NOTIFY(uiControl *c, NMHDR *nm, LRESULT *lResult);
```

You'll notice that they each take three parameters and return a `BOOL`. The first parameter to each is the uiControl itself. The last parameter is the address of an `LRESULT`. If your handler returns non-`FALSE`, libui will return the value you store in that `LRESULT` as the return value for the message. Otherwise, libui will send the message down the subclass chain.

The second parameter is what's different. In the case of `WM_COMMAND`, this is just the command code (equivalent to `HIWORD(wParam)` in a real `WM_COMMAND` handler). In the case of `WM_NOTIFY`, this is the `NMHDR` of the notification (equivalent to `(NMHDR *) lParam` in a real `WM_NOTIFY` handler).

The trackbar doesn't use `WM_COMMAND` at all, so we can simply return `FALSE` for that one:

```c
static BOOL onWM_COMMAND(uiControl *c, WORD code, LRESULT *lResult)
{
	return FALSE;
}
```

A trackbar *does* use `WM_NOTIFY`, however. For the purposes of this example, we won't be processing notifications. If you were to add some event, such as `OnChanged()`, you would handle that here (in this case, via `xxTODOxxx`).

```c
static BOOL onWM_NOTIFY(uiControl *c, NMHDR *nm, LRESULT *lResult)
{
	return FALSE;
}
```

ok TODO this was a bad example
