# Useful things in newer versions

## Windows
### Windows 7
http://channel9.msdn.com/blogs/pdc2008/pc43

TODO look up PDC 2008 talk "new shell user interface"

- new animation and text engine
- ribbon control (didn't this have some additional license?)
- LVITEM.piColFmt

### Windows 8

### Windows 8.1

### Windows 10

## GTK+
TODO what ships with Ubuntu Quantal (12.10)?

### GTK+ 3.6
ships with: Ubuntu Raring (13.04)

- GtkEntry and GtkTextView have input purposes and input hints for external input methods but do not change input themselves
	- according to Company, we connect to insert-text for that
- GtkLevelBar
- GtkMenuButton
- **GtkSearchEntry**

### GTK+ 3.8
ships with: Ubuntu Saucy (13.10)

Not many interesting new things to us here, unless you count widget-internal tickers and single-click instead of double-click to select list items (a la KDE)... and oh yeah, also widget opacity.

### GTK+ 3.10
ships with: **Ubuntu Trusty (14.04 LTS)**
<br>GLib version: 2.40

- tab character stops in GtkEntry
- GtkHeaderBar
	- intended for titlebar overrides; GtkInfoBar is what I keep thinking GtkHeaderBar is
- **GtkListBox**
- GtkRevealer for smooth animations of disclosure triangles
- GtkSearchBar for custom search popups
- **GtkStack and GtkStackSwitcher**
- titlebar overrides (seems to be the hot new thing)

### GTK+ 3.12
ships with: Ubuntu Utopic (14.10)
<br>GLib version: 2.42

- GtkActionBar (basically like the bottom-of-the-window toolbars in Mac programs)
- gtk_get_locale_direction(), for internationalization
- more control over GtkHeaderBar
- **GtkPopover**
	- GtkPopovers on GtkMenuButtons
- GtkStack signaling
- **gtk_tree_path_new_from_indicesv()** (for when we add Table if we have trees too)

### GTK+ 3.14
ships with: **Debian Jessie**, Ubuntu Vivid (15.04)
<br>GLib version: Debian: 2.42, Ubuntu: 2.44

- gestures
- better GtkListbox selection handling
- more style classes (TODO also prior?)
- delayed switch changes on GtkSwitch

### GTK+ 3.16
ships with: Ubuntu Wily (15.10)
<br>GLib version: 2.46

- gtk_clipboard_get_default() (???)
- **GtkGLArea**
- proper xalign and yalign for GtkLabel; should get rid of runtime deprecation warnings
- better control of GtkListBox model-based creation (probably not relevant but)
- GtkModelButton (for GActions; probably not relevant?)
- wide handles on GtkPaned
- GtkPopoverMenu
- IPP paper names in GtkPaperSize (TODO will this be important for printing?)
- multiple matches in GtkSearchEntry (TODO evaluate priority)
- **GtkStackSidebar**
-  GTK_STYLE_CLASS_LABEL, GTK_STYLE_CLASS_MONOSPACE, GTK_STYLE_CLASS_STATUSBAR, GTK_STYLE_CLASS_TOUCH_SELECTION, GTK_STYLE_CLASS_WIDE (TODO figure out which of these are useful)
- GtkTextView: extend-selection
- GtkTextView: font fallbacks

### GTK+ 3.18

### GTK+ 3.20

## Cocoa
### Mac OS X 10.8

- Foundation ([full details](https://developer.apple.com/library/mac/releasenotes/Foundation/RN-FoundationOlderNotes/#//apple_ref/doc/uid/TP40008080-TRANSLATED_CHAPTER_965-TRANSLATED_DEST_999B))
	- NSDateComponents supports leap months
	- NSNumberFormatter and NSDateFormatter default to 10.4 behavior by default (need to explicitly do this on 10.7)
	- **NSUserNotification and NSUserNotificationCenter for Growl-style notifications**
	- better linguistic triggers for Spanish and Italian
	- NSByteCountFormatter
- AppKit ([full details](https://developer.apple.com/library/mac/releasenotes/AppKit/RN-AppKitOlderNotes/#X10_8Notes))
	- view-based NSTableView/NSOutlineView have expansion tooltips
	- NSScrollView magnification
	- Quick Look events; TODO see if they conflict with keyboard handling in Area
	- NSPageController (maybe useful?)
	- not useful for package UI, but may be useful for a new library (probably not by me): NSSharingService
	- NSOpenPanel and NSSavePanel are now longer NSPanels or NSWindows in sandboxed applications; this may be an issue should anyone dare to enable sandboxing on a program that uses package ui
	- NSTextAlternatives
	- -[NSOpenGLContext setFullScreen] now ineffective
	- +[NSColor underPageBackgroundColor]

### Mac OS X 10.9

- Foundation ([full details](https://developer.apple.com/library/mac/releasenotes/Foundation/RN-Foundation/))
	- system-provided progress reporting/cancellation support
	- NSURLComponents
	- **NSCalendar, NSDateFormatter, and NSNumberFormatter are now thread-safe**
	- various NSCalendar and NSDateComponents improvements
- AppKit ([full details](https://developer.apple.com/library/mac/releasenotes/AppKit/RN-AppKit/))
	- sheet handling is now block-based, queued, and in NSWindow; the delegate-based NSApplication API will still exist, except without the queue
	- similar changes to NSAlert
	- **return value changes to NSAlert**
	- window visibility APIs (occlusion)
	- NSApplicationActivationPolicyAccessory
	- fullscreen toolbar behavior changes
	- status items for multiple menu bars
	- better NSSharingService support
	- a special accelerated scrolling mode, Responsive Scrolling; won't matter for us since I plan to support the scroll wheel and it won't
	- NSScrollView live scrolling notifications
	- NSScrollView floating (anchored/non-scrolling) subviews
	- better multimonitor support
	- better key-value observing for NSOpenPanel/NSSavePanel (might want to look this up to see if we can override some other juicy details... TODO)
	- better accessory view key-view handling in NSOpenPanel/NSSavePanel
	- NSAppearance
	- **-[NSTableView moveRowAtIndex:toIndex:] bug regarding first responders fixed**
	- view-specific RTL overrides

### Mac OS X 10.10

### Mac OS X 10.11
* **NSLayoutGuide**
