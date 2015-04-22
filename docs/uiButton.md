# uiButton

uiButton is a control which represents a clickable button.

Windows | Unix | OS X
-----|-----|-----
![Button on Windows](images/uiButton_windows.png)|![Button on Unix](images/uiButton_unix.png)|![Button on OS X](images/uiButton_darwin.png)

uiButton derives from [uiControl](uiControl.md).

## constructor NewButton()
```c
uiButton *uiNewButton(const char *text);
```
Creates a new uiButton with the specifed text.

## func Text()
```c
char *uiButtonText(uiButton *b);
```
Returns the text shown on the uiButton. Free the returned string with `uiTextFree()`.

## func SetText()
```c
void uiButtonSetText(uiButton *b, const char *text);
```
Changes the text shown on the uiButton to the given text string.

## func OnClicked()
```c
void uiButtonOnClicked(uiButton *b, void (*handler)(void *sender, void *data), void *data);
```
Sets the function that is called when the user clicks the uiButton. If a handler was previously assigned, this call replaces the old handler with the given one.

The `sender` argument to the callback is the `b` argument to `uiButtonOnClicked()`. It is of type `void *` to allow uiMenuItems to use the same callback functions.

The `data` argument to the callback is the `data` argument to `uiButtonOnClicked()`.

The default handler does nothing.

TODO allow NULL to return to default?
