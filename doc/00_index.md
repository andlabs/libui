<!-- 8 april 2019 -->

# Using libui

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

*before* including `ui.h`, as that informs `ui.h` to tell the compiler that the functions in `ui.h` are not dynamically loaded.

TODO(andlabs): talk about OS-specific headers

## Linking Against libui

TODO(andlabs): write this
