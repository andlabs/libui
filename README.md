# libui: a portable GUI library for C

This README is being written.<br>
[![Build Status](https://travis-ci.org/andlabs/libui.png)](https://travis-ci.org/andlabs/libui)

## Announcements

* **24 May 2016**
	* You can now help choose [a potential new build system for libui](https://github.com/andlabs/libui/issues/62).
	* Tomorrow I will decide if OS X 10.7 will also be dropped alongside GTK+ 3.4-3.8 this Saturday. Stay tuned.

* **22 May 2016**
	* Two more open questions I'd like your feedback on are available [here](https://github.com/andlabs/libui/issues/48) and [here](https://github.com/andlabs/libui/issues/25).
	* Sometime in the next 48 hours (before 23:59 EDT on 24 May 2016) I will split `uiCombobox` into two separate controls, `uiCombobox` and `uiEditableCombobox`, each with slightly different events and "selected item" mechanics. Prepare your existing code.

* **21 May 2016**
	* I will now post announcements and updates here.
	* Now that Ubuntu 16.04 LTS is here, no earlier than next Saturday, 28 May 2016 at noon EDT, **I will bump the minimum GTK+ version from 3.4 to 3.10**. This will add a lot of new features that I can now add to libui, such as search-oriented uiEntries, lists of arbitrary control layouts, and more. If you are still running a Linux distribution that doesn't come with 3.10, you will either need to upgrade or use jhbuild to set up a newer version of GTK+ in a private environment.
	* You can decide if I should also drop OS X 10.7 [here](https://github.com/andlabs/libui/issues/46).

## Updates

*Note that today's entry may be updated later today.*

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

## Runtime Requirements

* Windows: Windows Vista SP2 with Platform Update or newer
* Unix: GTK+ 3.4 or newer
* Mac OS X: OS X 10.7 or newer

## Build Requirements

* All platforms:
	* GNU make 3.81 or newer (Xcode comes with this; on Windows you will need to get it yourself)
* Windows: Microsoft Visual Studio 2013 or newer (2013 is needed for `va_copy()`)
	* MinGW is currently unsupported. MinGW-w64 support will be re-added once the following features come in:
		* [Isolation awareness](https://msdn.microsoft.com/en-us/library/aa375197%28v=vs.85%29.aspx)
		* Linker symbols for some functions such as `TaskDialog()` (which I thought I submitted...)
* Unix: nothing specific
* Mac OS X: nothing specific, so long as you can build Cocoa programs

(TODO write some notes on make variables and cross-compiling)

## Documentation

Needs to be written. Consult ui.h and the examples for details for now.

## Language Bindings

libui was originally written as part of my [package ui for Go](https://github.com/andlabs/ui). Now that libui is separate, package ui has become a binding to libui. As such, package ui is the only official binding.

Other people have made bindings to other languages:
* TODO list them here

## Screenshots

From examples/controlgallery:

![Windows](examples/controlgallery/windows.png)

![Unix](examples/controlgallery/unix.png)

![OS X](examples/controlgallery/darwin.png)
