TODO clean this up

TODO note that you -fvisibility=hidden means nothing in static libraries, hence this (confirmed on OS X)

In general, all names that begin with "ui" and are followed by a capital letter and all names htat begin with "uipriv" and are followed by a capita lletter are reserved by libui. This applies even in C++, where name mangling may affect the actual names in the object file.

# Reserved names; for users

All reserved names in libui are defined by a prefix followed by any uppercase letter in ASCII. The bullet lists before list those prefixes.

Global-scope identifiers of any form (variables, constant names, functions, structure names, union names, C++ class names, enum type names, enum value names, C++ namespace names, GObject class and interface struct names, and Objective-C class and protocol name identifiers) and macro names:

- `ui`
- `uipriv`

GObject and Objective-C class, interface, and protocol name strings, in the form they take in their respective runtime memory (e.g. when passed to `g_type_from_name()` and `NSClassFromString()`, respectively):

- `uipriv`

Objective-C method names:

- `initWithUipriv`
- `initWithFrame:uipriv` (TODO probably worth removing)
- `uipriv`
- `isUipriv` (for compatibility with KVO and `@property` statements)
- `setUipriv` (for compatibility with KVO and `@property` statements)

Objective-C ivar names:

- `uipriv`
- `_uipriv` (for compatibility with KVO and `@property` statements)

Objective-C property names:

- `uipriv`

TODO GObject macros (in libui's source code), properties, and signals

# Developer notes

TODO
