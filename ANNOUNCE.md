# Old Announcements

* **29 May 2016**
	* **Alpha 3 is here!** Get it [here](https://github.com/andlabs/libui/releases/tag/alpha3).
	* The next packaged release will introduce:
		* uiGrid, another way to lay out controls, a la GtkGrid
		* uiOpenGLArea, a way to render OpenGL content in a libui uiArea
		* uiTable, a data grid control that may or may not have tree facilities (if it does, it will be called uiTree instead)
		* a complete, possibly rewritten, drawing and text rendering infrastructure

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
