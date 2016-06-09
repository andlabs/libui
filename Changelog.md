# Old Updates

* **29 May 2016**
	* Thanks to @pcwalton, we can now statically link libui! Simply do `make STATIC=1` instead of just `make`.
		* On Windows you must link both `libui.lib` and `libui.res` AND provide a Common Controls 6 manifest for output static binaries to work properly.

* **28 May 2016**
	* As promised, **the minimum system requirements are now OS X 10.8 and GTK+ 3.10 for OS X and Unix, respectively**.

* **26 May 2016**
	* Two OS X-specific functions have been added: `uiDarwinMarginAmount()` and `uiDarwinPaddingAmount()`. These return the amount of margins and padding, respectively, to give to a control, and are intended for container implementations. These are suitable for the constant of a NSLayoutConstraint. They both take a pointer parameter that is reserved for future use and should be `NULL`.

* **25 May 2016**
	* uiDrawTextLayout attributes are now specified in units of *graphemes* on all platforms. This means characters as seen from a user's perspective, not Unicode codepoints or UTF-8 bytes. So a long string of combining marker codepoints after one codepoint would still count as one grapheme.

* **24 May 2016**
	* As promised, `uiCombobox` is now split into `uiCombobox` for non-editable comboboxes and `uiEditableCombobox` for editable comboboxes. Mind the function changes as well :)
	* There is a new function `uiMainStep()`, which runs one iteration of the main loop. It takes a single boolean argument, indicating whether to wait for an event to occur or not. It returns true if an event was processed (or if no event is available if you don't want to wait) and false if the event loop was told to stop (for instance, `uiQuit()` was called).

* **23 May 2016**
	* Fixed surrogate pair drawing on OS X.

* **22 May 2016**
	* Removed `uiControlVerifyDestroy()`; that is now part of `uiFreeControl()` itself.
	* Added `uiPi`, a constant for π. This is provided for C and C++ programmers, where there is no standard named constant for π; bindings authors shouldn't need to worry about this.
	* Fixed uiMultilineEntry not properly having line breaks on Windows.
	* Added `uiNewNonWrappingMultilineEntry()`, which creates a uiMultilineEntry that scrolls horizontally instead of wrapping lines. (This is not documented as being changeable after the fact on Windows, hence it's a creation-time choice.)
	* uiAreas on Windows and some internal Direct2D areas now respond to `WM_PRINTCLIENT` properly, which should hopefully increase the quality of screenshots.
	* uiDateTimePicker on GTK+ works properly on RTL layouts and no longer disappears off the bottom of the screen if not enough room is available. It will also no longer be marked for localization of the time format (what the separator should be and whether to use 24-hour time), as that information is not provided by the locale system. :(
	* Added `uiUserBugCannotSetParentOnToplevel()`, which should be used by implementations of toplevel controls in their `SetParent()` implementations. This will also be the beginning of consolidating common user bug messages into a single place, though this will be one of the only few exported user bug functions.
	* uiSpinbox and uiSlider now merely swap their min and max if min ≥ max. They will no longer panic and do nothing, respectively.
	* Matrix scaling will no longer leave the matrix in an invalid state on OS X and GTK+.
	* `uiMultilineEntrySetText()` and `uiMutlilineEntryAppend()` on GTK+ no longer fire `OnChanged()` events.
