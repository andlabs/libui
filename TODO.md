- http://blogs.msdn.com/b/oldnewthing/archive/2004/01/12/57833.aspx provide a DEF file on Windows

- all ports: update state when adding a control to a parent
- should uiWindowsSizing be computed against the window handle, not the parent?

- DPI awareness on windows

- consider calling setAppleMenu: for the application menu; it doesn't seem to make much of a difference but
- http://stackoverflow.com/questions/4543087/applicationwillterminate-and-the-dock-but-wanting-to-cancel-this-action

ultimately:
- MAYBE readd lifetime handling/destruction blocking
- related? [12:25] <ZeroOne> And the blue outline on those buttons [ALL clicked buttons on Windows 7] won't go away
	- I get this too
- SWP_NOCOPYBITS to avoid button redraw issues on Windows when not in tab, but only when making resize faster
- secondary side alignment control in uiBox
- generate libui.lib and related files
- Windows: don't abort if a cleanup function fails?

- 32-bit Mac OS X support (requires lots of code changes)
	- change the build system to be more receptive to arch changes

notes to self
- explicitly document label position at top-left corner
- explicitly document that if number of radio buttons >= 1 there will always be a selection
- mark that uiControlShow() on a uiWindow() will bring to front and give keyboard focus because of OS X
	- make sure ShowWindow() is sufficient for zorder on Windows
- document that you CAN use InsertAt functions to insert at the first invalid index, even if the array is empty
- note that uiTabInsertAt() does NOT change the current tab page (it may change its index if inserting before the current page)
- note that the default action for uiWindowOnClosing() is to return 0 (keep the window open)
- note that uiInitOptions should be initialized to zero
- explicitly document that uiCheckboxSetChecked() and uiEntrySetText() do not fire uiCheckboxOnToggled() and uiEntryOnChanged(), respectively
- note that if a menu is requested on systems with menubars on windows but no menus are defined, the result is a blank menubar, with whatever that means left up to the OS to decide
- note that handling of multiple consecutive separators in menus, leading separators in menus, and trailing separators in menus are all OS-defined

- test RTL
	- automate RTL

- http://blogs.msdn.com/b/oldnewthing/archive/2014/02/26/10503148.aspx

notes
- http://blogs.msdn.com/b/oldnewthing/archive/2004/03/29/101121.aspx on accelerators
