- change all private names to uipXxxx
- make it so Windows API calls that do logLastError(), etc. abort whatever they're doing and not try to continue, just like wintable
- figure out what to cleanup in darwin terminate:
	- delegate
	- deleted objects view

ultimately:
- make everything vtable-based
	- provide macros for the vtables
	- figure out where updateParent() plays into this
		- figure out what to do about custom containers
			- rename container to parent?
		- make the code flow of all platforms fully symmetrical
