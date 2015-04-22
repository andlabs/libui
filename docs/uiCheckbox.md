# uiCheckbox

uiCheckbox is a control which represents a box that can have a checkmark in it.

Windows | Unix | OS X
-----|-----|-----
![Checkbox on Windows](images/uiCheckbox_windows.png)|![Checkbox on Unix](images/uiCheckbox_unix.png)|![Checkbox on OS X](images/uiChekbox_darwin.png)

uiCheckbox derives from [uiControl](uiControl.md).

## constructor NewCheckbox()
```c
uiCheckbox *uiNewCheckbox(const char *text);
```
Creates a new uiCheckbox with the specifed text.

## func Text()
```c
char *uiCheckboxText(uiCheckbox *c);
```
Returns the text shown on the uiCheckbox. Free the returned string with `uiTextFree()`.

## func SetText()
```c
void uiCheckboxSetText(uiCheckbox *c, const char *text);
```
Changes the text shown on the uiCheckbox to the given text string.

## func OnToggled()
```c
void uiCheckboxOnToggled(uiCheckbox *c, void (*handler)(void *sender, void *data), void *data);
```
Sets the function that is called when the user changes the checked state of the uiCheckbox. If a handler was previously assigned, this call replaces the old handler with the given one.

The `sender` argument to the callback is the `c` argument to `uiCheckboxOnToggled()`. It is of type `void *` to allow uiMenuItems to use the same callback functions.

The `data` argument to the callback is the `data` argument to `uiCheckboxOnToggled()`.

The default handler does nothing.

TODO allow NULL to return to default?

## func Checked()
```c
int uiCheckboxChecked(uiCheckbox *c);
```
Returns nonzero if the given uiCheckbox is currently chcecked, and zero otherwise.

## func SetChecked()
```c
void uiCheckboxSetChecked(uiCheckbox *c, int checked)
```
Changes the checked state of the given uiCheckbox. If `checked` is nonzero, the checkbox is checked. If zero, the checkbox is unchecked.

Calling `uiCheckboxSetChecked()` does NOT trigger the `OnToggled()` event.
