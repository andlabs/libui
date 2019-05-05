<!-- 8 april 2019 -->

# Using libui

>**Note**: This page discusses instructions for compiling and linking libui, and does so in the most general way possible. Your build system may provide better options for doing any of the following than what is described on this page; consult its manual.

## Shared vs. Static

In order to properly use libui, you first need to know whether you are using it as a shared library (also called a dynamically-linked library) or as a static library.

With a shared library, your application will need to ship with the libui shared library object, or require it at runtime (if it is to be installed via a system package manager). However, your program will only need to directly depend on libui itself.

With a static library, your application will ship with libui embedded within it, so no additional dependencies will be needed at runtime (apart from ones that require installation via a system package manager, such as GTK+). However, you need to do slightly more work to be able to actually build your application; exactly what is discussed on this page.

If you are using a language binding, the language binding will likely have already decided how it will use libui, and will provide its own instructions on what to do based on that decision.

## Including libui in Source Code

libui ships as a single header file, `ui.h`. Therefore, to include it in a C or C++ source file, all you need to do is

```c
// If the libui headers are loaded in the same way as other system headers
#include <ui.h>
// OR
// If the libui headers are included directly
#include "path/to/ui.h"
```

If you are using libui as a static library, you'll need to add the line

```c
#define uiStatic
```

*before* including `ui.h`, as that informs `ui.h` to tell the compiler that the functions in `ui.h` are not dynamically loaded. (You may, of course, also do this on your compiler's command line.)

### OS-Specific Headers

For most purposes, the above will be sufficient. However, if you need to do any OS-specific work with libui, there are a few more steps to take. Typically, this would be done if you either wanted to create a new control or access the underlying OS handles behind a control.

Each OS has a special OS-specific header that provides the necessary additional functions and constants. These must be included *after* `ui.h`. These must also be included *after* any OS headers:

#### Windows

The OS-specific header is `ui_windows.h`. The only OS header that is necessary is `<windows.h>`. You should also specify the Unicode, type-strictness, and minimum-version macros before including `<windows.h>`:

```c
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
// This sets the minimum Windows API version to the minimum for libui: Windows Vista.
// Feel free to make this higher if your code will not run on all versions libui supports.
#define WINVER         0x0600
#define _WIN32_WINNT   0x0600
#define _WIN32_WINDOWS 0x0600
#define _WIN32_IE      0x0700
#define NTDDI_VERSION  0x06000000
#include <windows.h>
#include "ui.h"
#include "ui_windows.h"
```

#### Unix

The OS-specific header is `ui_unix.h`. Only `<gtk/gtk.h>` needs to be included beforehand:

```c
#include <gtk/gtk.h>
#include "ui.h"
#include "ui_unix.h"
```

TODO(andlabs): version constants

When actually running your compiler, you'll need to tell it where to look for GTK+. The preferred way to do so is via `pkg-config`. For instance:

```shell
$ gcc -c -o program.o program.c `pkg-config --cflags gtk+-3.0`
```

#### macOS

The OS-specific header is `ui_darwin.h`. Only `<Cocoa/Cocoa.h>` needs to be included beforehand:

```objective-c
#import <Cocoa/Cocoa.h>
#import "ui.h"
#import "ui_darwin.h"
```

TODO(andlabs): is this really sufficient, or is only AppKit/Foundation necessary? Or just Foundation?

## Linking Against libui

How to link against libui depends on how you are using libui.

### Linking Against libui as a Shared Library

When linking a shared library, all you need is the shared library itself, and the compiler-specific mechanism for linking shared libraries applies. Note that for MSVC, this means you link against `libui.lib`, not `libui.dll`!

### Linking Against libui as a Static Library

Because static libraries are merely collections of object files, in order to link against libui as a static library, you will also need to link against libui's dependencies.

#### Windows

libui needs to be linked against a set of standard Windows DLLs, preferably in the order listed here. The syntax to use depends on the compiler; examples are listed for the first one, and all follow the same pattern.

- user32.dll (MSVC: `user32.lib`, MinGW: `-luser32`)
- kernel32.dll
- gdi32.dll
- comctl32.dll
- TODO

**Furthermore**, because libui requires Common Controls v6, you will also need to provide a manifest file that specifies Common Controls v6, and ideally also speciifes the versions of Windows your executable runs on. For instance, the one used by libui.dll is a good place to start:

```xml
TODO
```

Ideally, you should provide this as a resource that's linked into your program:

```rc
// Extra #defines above elided for brevity; these should be included too.
#include <windows.h>

CREATEPROCESS_MANIFEST_RESOURCE_ID RT_MANIFEST "manifest.xml"
```

Pass this file into `rc` (MSVC)/`windres` (MinGW) and then pass the output into your linker.

With MinGW-w64, you must link as C++ (for instance, with `g++`), as libui is written in C++ on Windows.

#### Unix

At a minimum, you'll need to link against GTK+ itself. As with compiling, the preferred way to get the correct linker flags is with `pkg-config`. For instance:

```shell
$ gcc -o program program.o `pkg-config --libs gtk+-3.0`
```

Depending on your system, you'll also need to link against libm and libdl; your linker will complain in some way. For instance, if you see something like `DSO missing from command line`, add `-ldl` to make libdl explicit.

#### macOS

You'll need to link against the `Foundation` and `AppKit` frameworks. To do so, simply pass `-framework Foundation` and `-framework AppKit` to your linker command.
