- make sure the last line of text layouts include leading

- documentation notes:
	- static binaries do not link system libraries, meaning apps still depend on shared GTK+, etc.
	- ui*Buttons are NOT compatible with uiButton functions

- more robust layout handling
	- uiFormTie() for ensuring multiple uiForms have the same label area widths
	- uiSizeGroup for size groups (GtkSizeGroup on GTK+, auto layout constraints on OS X; consider adding after 10.8 is gone)

- windows: should the initial hwndInsertAfter be HWND_BOTTOM for what we want?

- windows: document the rules for controls and containers

- windows: document the minimum size change propagation system

- provisions for controls that cannot be grown? especiailly for windows

- LC_VERSION_MIN_MACOSX has the 10.11 SDK; see if we can knock it down to 10.8 too; OS version is fine
	- apply the OS version stuff to the test program and examples too
	- what about micro versions (10.8.x)? force 10.8.0?

- go through ALL the objective-c objects we create and make sure we are using the proper ownership (alloc/init and new are owned by us, all class method constructors are autoreleased - thanks mikeash)

- on OS X, edit shortcuts like command-C working require that the menu entries be defined, or so it seems, even for NSAlert
	- other platforms?

- make sure all OS X event handlers that use target-action set the action to NULL when the target is unset

- provide a way to get the currently selected uiTab page? set?

- make it so that the windows cntrols only register a resize if their new minimum size is larger than their current size to easen the effect of flicker
	- it won't remove that outright, but it'll help

- add an option to the test program to run page7b as an independent test in its own window
	- same for page7c

- http://blogs.msdn.com/b/oldnewthing/archive/2004/01/12/57833.aspx provide a DEF file on Windows

- all ports: update state when adding a control to a parent
- should uiWindowsSizing be computed against the window handle, not the parent?

- DPI awareness on windows

- http://stackoverflow.com/questions/4543087/applicationwillterminate-and-the-dock-but-wanting-to-cancel-this-action

ultimately:
- MAYBE readd lifetime handling/destruction blocking
- related? [12:25] <ZeroOne> And the blue outline on those buttons [ALL clicked buttons on Windows 7] won't go away
	- I get this too
		- not anymore
- SWP_NOCOPYBITS to avoid button redraw issues on Windows when not in tab, but only when making resize faster
- secondary side alignment control in uiBox
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
- note that uiDrawMatrixInvert() does not change the matrix if it fails
- note that the use of strings that are not strictly valid UTF-8 results in undefined behavior

- test RTL
	- automate RTL
- now that stock items are deprecated, I have to maintain translations of the Cancel, Open, and Save buttons on GTK+ myself (thanks baedert in irc.gimp.net/#gtk+)
	- either that or keep using stock items

- http://blogs.msdn.com/b/oldnewthing/archive/2014/02/26/10503148.aspx

- build optimizations

- use http://www.appveyor.com/ to do Windows build CI since people want CI






- consider just having the windows backend in C++
	- consider having it all in C++



don't forget LONGTERMs as well

notes
- http://blogs.msdn.com/b/oldnewthing/archive/2004/03/29/101121.aspx on accelerators

- group and tab should act as if they have no child if the child is hidden
on windows



- a way to do recursive main loops
	- how do we handle 0 returns from non-recursive uiMainStep() calls that aren't the main loop? (event handlers, for instance)
- should repeated calls to uiMainStep() after uiQuit() return 0 reliably? this will be needed for non-recursive loops

http://stackoverflow.com/questions/38338426/meaning-of-ampersand-in-rc-files/38338841?noredirect=1#comment64093084_38338841

label shortcut keys

- remove whining from source code

[01:41:47]  <vrishab>	Hi. does pango support "fgalpha". I see that foreground="112233xx" works ( alpha=xx ), but fgalpha is a no-op
[01:52:29]  <vrishab>	pango_attr_foreground_alpha_new (32767) seems to be called in either case, but only the "foreground" attr works
[01:56:09] 	lolek (lolek@ip-91-244-230-76.simant.pl) joined the channel
[01:57:48]  <vrishab>	ok. seems like "foreground" is mandatory attr, 1. "foreground-without-alpha" + "alpha" works 2. "foreground-with-alpha" works. 3. "alpha" alone doesn
[01:57:52]  <vrishab>	't work
[01:58:29]  <vrishab>	Is there a way to just specify alpha on the current foreground color ?
[02:00:23] 	lolek (lolek@ip-91-244-230-76.simant.pl) left the channel
[02:07:41] 	mjog (mjog@uniwide-pat-pool-129-94-8-98.gw.unsw.edu.au) left IRC (Quit: mjog)
[02:08:10] 	seb128 (seb128@53542B83.cm-6-5a.dynamic.ziggo.nl) joined the channel
[02:12:37]  <andlabs>	huh
[02:12:41]  <andlabs>	what version of pango?
[02:13:05]  <vrishab>	the latest .
[02:15:00]  <vrishab>	1.40.3
[02:20:46]  <andlabs>	I'll ahve to keep this in mind then, thanks
[02:20:59]  <andlabs>	if only there was a cairo-specific attribute for alpha...

FONT LOADING

[00:10:08]  <hergertme>	andlabs: is there API yet to load from memory? last i checked i only found from file (which we use in builder). https://git.gnome.org/browse/gnome-builder/tree/libide/editor/ide-editor-map-bin.c#n115
[00:13:12] 	mrmcq2u_ (mrmcq2u@109.79.53.90) joined the channel
[00:14:59] 	mrmcq2u (mrmcq2u@109.79.73.102) left IRC (Ping timeout: 181 seconds)
[00:15:19]  <andlabs>	hergertme: no, which is why I was asking =P
[00:15:30]  <andlabs>	I would have dug down if I could ensure at least something about the backends a GTK+ 3 program uses
[00:15:39]  <andlabs>	on all platforms except windows and os x
[00:16:11]  <hergertme>	to the best of my (partially outdated, given pace of foss) knowledge there isn't an api to load from memory
[00:16:28]  <hergertme>	you can possibly make a tmpdir and put a temp file in there
[00:16:52]  <hergertme>	and load that as your font dir in your FcConfig, so any PangoFontDescription would point to that one font, no matter what
[00:17:18]  <hergertme>	(using the API layed out in that link)
[00:18:18] 	dsr1014__ (dsr1014@c-73-72-102-18.hsd1.il.comcast.net) joined the channel
[00:35:18] 	simukis_ (simukis@78-60-58-6.static.zebra.lt) left IRC (Quit: simukis_)
[00:35:48] 	dreamon_ (dreamon@ppp-188-174-49-41.dynamic.mnet-online.de) joined the channel
[00:40:09] 	samtoday_ (samtoday@114-198-116-132.dyn.iinet.net.au) joined the channel
[00:40:32] 	mjog (mjog@120.18.225.46) joined the channel
[00:40:38]  <andlabs>	hergertme: not necessarily fontconfig
[00:40:45]  <andlabs>	it can be with ft2 or xft I guess
[00:40:55]  <andlabs>	especially since I want the API NOT to make the font part of the font panel
[00:42:07]  <hergertme>	what sort of deprecated code are you trying to support?
[00:42:35]  <hergertme>	both of those are deprecated in pango fwiw
[00:43:06]  <hergertme>	on Linux im pretty sure we use FC everywhere these days
[00:44:46]  <hergertme>	(and gtk_widget_set_font_map() is how you get your custom font into a widget without affecting the global font lists, as layed out in that link)
[00:49:14] 	vasaikar (vasaikar@125.16.97.121) joined the channel
[00:50:14] 	karlt (karl@2400:e780:801:224:f121:e611:d139:e70e) left IRC (Client exited)
[00:50:49] 	karlt (karl@2400:e780:801:224:f121:e611:d139:e70e) joined the channel
[00:51:43] 	PioneerAxon (PioneerAxo@122.171.61.146) left IRC (Ping timeout: 180 seconds)
[00:57:47] 	PioneerAxon (PioneerAxo@106.201.37.181) joined the channel
[01:03:01] 	karlt (karl@2400:e780:801:224:f121:e611:d139:e70e) left IRC (Ping timeout: 181 seconds)
[01:05:49] 	muhannad (muhannad@95.218.26.152) left IRC (Quit: muhannad)
[01:07:51]  <andlabs>	hergertme: hm
[01:07:54]  <andlabs>	all right, thanks
[01:08:05]  <andlabs>	hergertme: fwiw right now my requirement is 3.10
[01:10:47]  <hergertme>	ah, well you'll probably be missing the neccesary font API on gtk_widget
[01:11:04]  <hergertme>	but pango should be fine even back as far as https://developer.gnome.org/pango/1.28/PangoFcFontMap.html
[01:11:56]  <andlabs>	good
[01:12:04]  <andlabs>	because this is for custom drawing into a DrawingArea
[01:14:12]  <hergertme>	presumably just create your PangoContext as normal, but call pango_context_set_font_map() with the map you've setup. now, the load a font from a file i dont think was added to FontConfig until later though (not sure what release)
[01:15:53]  <hergertme>	FcConfigAppFontAddFile() <-- that API
[01:16:30]  <hergertme>	great, and they don't say what version the API was added in teh docs
function: ide_editor_map_bin_add()

- Mouse ClickLock: do we need to do anything special? *should* we? https://msdn.microsoft.com/en-us/library/windows/desktop/ms724947(v=vs.85).aspx
- consider a uiAnticipateDoubleClick() or uiDoubleClickTime() (for a uiQueueTimer()) or something: https://blogs.msdn.microsoft.com/oldnewthing/20041015-00/?p=37553

- determine whether MSGF_USER is for and if it's correct for our uiArea message filter (if we have one)

- source file encoding and MSVC compiler itself? https://stackoverflow.com/questions/20518040/how-can-i-get-the-directwrite-padwrite-sample-to-work
	- also need to worry about object file and output encoding...
	- this also names the author of the padwrite sample

- OpenType features TODOs
	- https://stackoverflow.com/questions/32545675/what-are-the-default-typography-settings-used-by-idwritetextlayout
	- feature/shaping interaction rules for arabic: https://www.microsoft.com/typography/OpenTypeDev/arabic/intro.htm
	- other stuff, mostly about UIs and what users expect to be able to set
		- https://klim.co.nz/blog/towards-an-ideal-opentype-user-interface/
		- https://libregraphicsmeeting.org/2016/designing-for-many-applications-opentype-features-ui/
			- https://www.youtube.com/watch?v=wEyDhsH076Y
			- https://twitter.com/peter_works
		- http://ilovetypography.com/2014/10/22/better-ui-for-better-typography-adobe-petition/
		- http://silgraphite.sourceforge.net/ui/studynote.html

- add NXCOMPAT (DEP awareness) to the Windows builds
	- and ASLR too? or is that not a linker setting

OS X: embedding an Info.plist into a binary directly
https://www.objc.io/issues/6-build-tools/mach-o-executables/
TODO will this let Dictation work?
TODO investigate ad-hoc codesigning

https://blogs.msdn.microsoft.com/oldnewthing/20040112-00/?p=41083 def files for decoration (I forget if I said this earlier)

TODO ClipCursor() stuff; probably not useful for libui but still
https://blogs.msdn.microsoft.com/oldnewthing/20140102-00/?p=2183
https://blogs.msdn.microsoft.com/oldnewthing/20061117-03/?p=28973
https://msdn.microsoft.com/en-us/library/windows/desktop/ms648383(v=vs.85).aspx

https://cmake.org/Wiki/CMake_Useful_Variables
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
On Unix systems, this will make linker report any unresolved symbols from object files (which is quite typical when you compile many targets in CMake projects, but do not bother with linking target dependencies in proper order). 
(I used to have something like this back when I used makefiles; did it convert in? I forget)

look into these for the os x port
https://developer.apple.com/documentation/appkit/view_management/nseditor?language=objc
https://developer.apple.com/documentation/appkit/view_management/nseditorregistration?language=objc

for future versions of the os x port
https://developer.apple.com/documentation/appkit/nslayoutguide?language=objc and anchors
https://developer.apple.com/documentation/appkit/nsuserinterfacecompression?language=objc https://developer.apple.com/documentation/appkit/nsuserinterfacecompressionoptions?language=objc
though at some point we'll be able to use NSStackView and NSGridView directly, so...

Cocoa PDFs
https://developer.apple.com/documentation/appkit/nspdfimagerep?language=objc
https://developer.apple.com/documentation/coregraphics?language=objc
https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/Printing/osxp_pagination/osxp_pagination.html#//apple_ref/doc/uid/20001051-119037
https://developer.apple.com/documentation/appkit/nsprintoperation/1529269-pdfoperationwithview?language=objc
https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/Printing/osxp_printapps/osxp_printapps.html#//apple_ref/doc/uid/20000861-BAJBFGED
https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/Printing/osxp_printingapi/osxp_printingapi.html#//apple_ref/doc/uid/10000083i-CH2-SW2
https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/Printing/osxp_printinfo/osxp_printinfo.html#//apple_ref/doc/uid/20000864-BAJBFGED
https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/Printing/osxp_printlayoutpanel/osxp_printlayoutpanel.html#//apple_ref/doc/uid/20000863-BAJBFGED
https://developer.apple.com/documentation/appkit/nspagelayout?language=objc
https://developer.apple.com/documentation/appkit/nsprintinfo?language=objc
https://developer.apple.com/documentation/applicationservices/core_printing?language=objc
https://developer.apple.com/documentation/applicationservices/1463247-pmcreatesession?language=objc
https://developer.apple.com/documentation/applicationservices/pmprintsession?language=objc
https://developer.apple.com/documentation/applicationservices/1460101-pmsessionbegincgdocumentnodialog?language=objc
https://developer.apple.com/documentation/applicationservices/1463416-pmsessionbeginpagenodialog?language=objc
https://developer.apple.com/documentation/applicationservices/1506831-anonymous/kpmdestinationprocesspdf?language=objc
https://developer.apple.com/documentation/applicationservices/1461960-pmcreategenericprinter?language=objc
https://developer.apple.com/documentation/applicationservices/1460101-pmsessionbegincgdocumentnodialog?language=objc
https://developer.apple.com/documentation/applicationservices/1464527-pmsessionenddocumentnodialog?language=objc
https://developer.apple.com/documentation/applicationservices/1461952-pmsessiongetcggraphicscontext?language=objc
https://developer.apple.com/library/content/technotes/tn2248/_index.html
https://developer.apple.com/library/content/samplecode/PMPrinterPrintWithFile/Introduction/Intro.html
https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/Printing/osxp_aboutprinting/osxp_aboutprt.html

- run os x code with `OBJC_DEBUG_MISSING_POOLS=YES` and other `OBJC_HELP=YES` options
	- turn off the autorelease pool to make sure we're not autoreleasing improperly

TODO investigate -Weverything in clang alongside -Wall in MSVC (and in gcc too maybe...)

mac os x accessibility
- https://developer.apple.com/documentation/appkit/nsworkspace/1524656-accessibilitydisplayshoulddiffer?language=objc
- https://developer.apple.com/documentation/appkit/nsworkspace/1526290-accessibilitydisplayshouldincrea?language=objc
- https://developer.apple.com/documentation/appkit/nsworkspace/1533006-accessibilitydisplayshouldreduce?language=objc

uiEntry disabling bugs http://www.cocoabuilder.com/archive/cocoa/215525-nstextfield-bug-can-be.html
uiMultilineEntry disabling https://developer.apple.com/library/content/qa/qa1461/_index.html

more TODOs:
- make no guarantee about buildability of feature branches
