# uiLabel

uiLabel is a control which represents a line of text that labels an element or area of a GUI.

Windows | Unix | OS X
-----|-----|-----
![Label on Windows](images/uiLabel_windows.png)|![Label on Unix](images/uiLabel_unix.png)|![Label on OS X](images/uiLabel_darwin.png)

uiLabel derives from [uiControl](uiControl.md).

## constructor NewLabel()
```c
uiLabel *uiNewLabel(const char *text);
```
Creates a new uiLabel with the specifed text.

## func Text()
```c
char *uiLabelText(uiLabel *l);
```
Returns the text shown on the uiLabel. Free the returned string with `uiTextFree()`.

## func SetText()
```c
void uiLabelSetText(uiLabel *l, const char *text);
```
Changes the text shown on the uiLabel to the given text string.
