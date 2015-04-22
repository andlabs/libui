# uiEntry

uiEntry is a control which represents an area where a user can enter a single line of text.

Windows | Unix | OS X
-----|-----|-----
![Entry on Windows](images/uiEntry_windows.png)|![Entry on Unix](images/uiEntry_unix.png)|![Entry on OS X](images/uiEntry_darwin.png)

uiEntry derives from [uiControl](uiControl.md).

## constructor NewEntry()
```c
uiEntry *uiNewEntry(void);
```
Creates a new uiEntry.

## func Text()
```c
char *uiEntryText(uiEntry *e);
```
Returns the text currently entered in the uiEntry. Free the returned string with `uiTextFree()`.

## func SetText()
```c
void uiEntrySetText(uiEntry *e, const char *text);
```
Changes the text currently entered in the uiEntry to the given text string.
