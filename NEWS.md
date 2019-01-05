# Old News

* **27 November 2016**
	* Decided to split the table stuff into its own branch. It will be developed independently of everything else, along with a few other features.

* **2 November 2016**
	* Added two new functions to replace the deleted `uiWindowPosition()` and friends: `uiAreaBeginUserWindowMove()` and `uiAreaBeginUserWindowResize()`. When used in a `uiAreaHandler.Mouse()` event handler, these let you initiate a user-driven mouse move or mouse resize of the window at any point in a uiArea.

* **31 October 2016**
	* @krakjoe noticed that I accidentally used thread-unsafe code in uiQueueMain() on Unix. Fixed.

* **24 October 2016**
	* `uiWindowSetContentSize()` on Unix no longer needs to call up the GTK+ main loop. As a result, bugs related to strange behavior using that function (and the now-deleted `uiWindowSetPosition()` and `uiWindowCenter()`) should go away. I'll need to go through the bugs to verify as much, though.

* **22 October 2016**
	* Due to being unable to guarantee they will work (especially as we move toward capability-driven window systems like Wayland), or being unable to work without hacking that breaks other things, the following functions have been removed: `uiWindowPosition()`, `uiWindowSetPosition()`, `uiWindowCenter()`, and `uiWindowOnPositionChanged()`. Centering may come back at some point in the future, albeit in a possibly restricted form. A function to initiate a user move when a part of a uiArea is clicked will be provided soon.

* **21 October 2016**
	* `uiDrawTextWeightUltraBold` is now spelled correctly. Thanks to @krakjoe.

* **18 June 2016**
	* Help decide [the design of tables and trees in libui](https://github.com/andlabs/libui/issues/159); the implementation starts within the next few days, if not tomorrow!

* **17 June 2016**
	* **CMake 3.1.0 is now required.** This is due to CMake's rapid development pace in the past few years adding things libui needs to build on as many systems as possible. If your OS is supported by libui but its repositories ship with an older version of CMake, you will need to find an updated one somewhere.
	* Please help [plan out a better menu API](https://github.com/andlabs/libui/issues/152).
	* `uiMainSteps()` no longer takes any arguments and no longer needs to invoke a function to do the work. You still need to call it, but once you do, it will return immediately and you can then get right to your main loop.
	* **CMake 3.1.0 is now required.** This is due to CMake's rapid development pace in the past few years adding things libui needs to build on as many systems as possible. If your OS is supported by libui but its repositories ship with an older version of CMake, you will need to find an updated one somewhere.
	* Added `uiNewVerticalSeparator()` to complement `uiNewHorizontalSeparator()`.

* **16 June 2016**
	* Added `uiWindowContentSize()`, `uiWindowSetContentSize()`, and `uiWindowOnContentSizeChanged()` methods for manipulating uiWindow content sizes. Note the use of "content size"; the size you work with does NOT include window decorations (titlebars, menus, etc.).
	* Added `uiWindowFullscreen()` and `uiWindowSetFullscreen()` to allow making fullscreen uiWindows, taking advantage of OS facilities for fullscreen and without changing the screen resolution (!).
	* Added `uiWindowBorderless()` and `uiWindowSetBorderless()` for allowing borderless uiWindows.
	* Added `uiMainSteps()`. You call this instead of `uiMain()` if you want to run the main loop yourself. You pass in a function that will be called; within that function, you call `uiMainStep()` repeatedly until it returns 0, doing whatever you need to do in the meantime. (This was needed because just having `uiMainStep()` by itself only worked on some systems.)
	* Added `uiProgressBarValue()` and allowed passing -1 to `uiProgressBarSetValue()` to make an indeterminate progress bar. Thanks to @emersion.

* **15 June 2016**
	* Added `uiFormDelete()`; thanks to @emersion.
	* Added `uiWindowPosition()`, `uiWindowSetPosition()`, `uiWindowCenter()`, and `uiWindowOnPositionChanged()`, methods for manipulating uiWindow position.

* **14 June 2016**
	* uiDarwinControl now has a `ChildVisibilityChanged()` method and a corresponding `NotifyVisibilityChanged()` function that is called by the default show/hide handlers. This is used to make visibility changes work on OS X; uiBox, uiForm, and uiGrid all respect these now.
	* The same has been done on the Windows side as well.
	* Hiding and showing controls and padding calculations are now correct on Windows at long last.
	* Hiding a control in a uiForm now hides its label on all platforms.

* **13 June 2016**
	* `intmax_t` and `uintmax_t` are no longer used for libui API functions; now we use `int`. This should make things much easier for bindings. `int` should be at least 32 bits wide; this should be sufficient for all but the most extreme cases.

* **12 June 2016**
	* Added `uiGrid`, a new container control that arranges controls in rows and columns, with stretchy ("expanding") rows, stretchy ("expanding") columns, cells that span rows and columns, and cells whose content is aligned in either direction rather than just filling. It's quite powerful, is it? =P

* **8 June 2016**
	* Added `uiForm`, a new container control that arranges controls vertically, with properly aligned labels on each. Have fun!

* **6 June 2016**
	* Added `uiRadioButtonsSelected()`, `uiRadioButtonsSetSelected()`, and `uiRadioButtonsOnSelected()` to control selection of a radio button and catch an event when such a thing happens.

* **5 June 2016**
	* **Alpha 3.1 is here.** This was a much-needed update to Alpha 3 that changes a few things:
		* **The build system is now cmake.** cmake 2.8.11 or higher is needed.
		* Static linking is now fully possible.
		* MinGW linking is back, but static only.
	* Added `uiNewPasswordEntry()`, which creates a new `uiEntry` suitable for entering passwords.
	* Added `uiNewSearchEntry()`, which creates a new `uiEntry` suitable for searching. On some systems, the `OnChanged()` event will be slightly delayed and/or combined, to produce a more natural feel when searching.

* **29 May 2016**
	* **Alpha 3 is here!** Get it [here](https://github.com/andlabs/libui/releases/tag/alpha3).
	* The next packaged release will introduce:
		* uiGrid, another way to lay out controls, a la GtkGrid
		* uiOpenGLArea, a way to render OpenGL content in a libui uiArea
		* uiTable, a data grid control that may or may not have tree facilities (if it does, it will be called uiTree instead)
		* a complete, possibly rewritten, drawing and text rendering infrastructure
	* Thanks to @pcwalton, we can now statically link libui! Simply do `make STATIC=1` instead of just `make`.
		* On Windows you must link both `libui.lib` and `libui.res` AND provide a Common Controls 6 manifest for output static binaries to work properly.

* **28 May 2016**
	* As promised, **the minimum system requirements are now OS X 10.8 and GTK+ 3.10 for OS X and Unix, respectively**.

* **26 May 2016**
	* Two OS X-specific functions have been added: `uiDarwinMarginAmount()` and `uiDarwinPaddingAmount()`. These return the amount of margins and padding, respectively, to give to a control, and are intended for container implementations. These are suitable for the constant of a NSLayoutConstraint. They both take a pointer parameter that is reserved for future use and should be `NULL`.

* **25 May 2016**
	* uiDrawTextLayout attributes are now specified in units of *graphemes* on all platforms. This means characters as seen from a user's perspective, not Unicode codepoints or UTF-8 bytes. So a long string of combining marker codepoints after one codepoint would still count as one grapheme.

* **24 May 2016**
	* You can now help choose [a potential new build system for libui](https://github.com/andlabs/libui/issues/62).
	* Tomorrow I will decide if OS X 10.7 will also be dropped alongside GTK+ 3.4-3.8 this Saturday. Stay tuned.
	* As promised, `uiCombobox` is now split into `uiCombobox` for non-editable comboboxes and `uiEditableCombobox` for editable comboboxes. Mind the function changes as well :)
	* There is a new function `uiMainStep()`, which runs one iteration of the main loop. It takes a single boolean argument, indicating whether to wait for an event to occur or not. It returns true if an event was processed (or if no event is available if you don't want to wait) and false if the event loop was told to stop (for instance, `uiQuit()` was called).

* **23 May 2016**
	* Fixed surrogate pair drawing on OS X.

* **22 May 2016**
	* Two more open questions I'd like your feedback on are available [here](https://github.com/andlabs/libui/issues/48) and [here](https://github.com/andlabs/libui/issues/25).
	* Sometime in the next 48 hours (before 23:59 EDT on 24 May 2016) I will split `uiCombobox` into two separate controls, `uiCombobox` and `uiEditableCombobox`, each with slightly different events and "selected item" mechanics. Prepare your existing code.
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

* **21 May 2016**
	* I will now post announcements and updates here.
	* Now that Ubuntu 16.04 LTS is here, no earlier than next Saturday, 28 May 2016 at noon EDT, **I will bump the minimum GTK+ version from 3.4 to 3.10**. This will add a lot of new features that I can now add to libui, such as search-oriented uiEntries, lists of arbitrary control layouts, and more. If you are still running a Linux distribution that doesn't come with 3.10, you will either need to upgrade or use jhbuild to set up a newer version of GTK+ in a private environment.
	* You can decide if I should also drop OS X 10.7 [here](https://github.com/andlabs/libui/issues/46).
