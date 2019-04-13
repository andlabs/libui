<!-- 12 april 2019 -->

# Initialization and the Main Loop

## Initialization

In order to use libui, you must first initialize it. All initialization is done through the `uiInit()` function. The thread that `uiInit()` is called on becomes the GUI thread. All of the functions of libui except `uiQueueMain()` **must** be called from this thread.

Furthermore, on some systems, this thread must also be the thread that `main()` is called on; macOS is the notable example here. Therefore, to be safe, you should only call `uiInit()` from `main()`, or from a function that runs on the same thread as `main()`. If you are using a language binding, that binding may provide further instruction on how to do so in that language.

`uiInit()` takes a set of options for setting up libui at a low level. There are no options at present, so this must currently be `NULL`.

`uiInit()` *can* fail, in which case libui is not safe to use. `uiInit()` will return a human-readable error message to the caller if it does fail. You are also responsible for both allocating and initializing the memory for this error message, since part of libui's initialization includes initialization of its memory allocators. The reference entry below has more information.

Here is an example of correct use of `uiInit()`:

```c
uiInitError err;

memset(&err, 0, sizeof (uiInitError));
err.Size = sizeof (uiInitError);
if (!uiInit(NULL, &err)) {
	fprintf(stderr, "error initializing libui: %s\n", err.Message);
	return 1;
}
```

Note that if `uiInit()` fails, you **cannot** use libui's message box functions to report the error.

## Reference

### `uiInit()`

```c
int uiInit(void *options, uiInitError *err);
```

### `uiUninit()`

```c
void uiUninit(void);
```

### `uiInitError`

```c
typedef struct uiInitError uiInitError;
struct uiInitError {
	size_t Size;
	char Message[256];
};
```

`uiInitError` describes an error returned by `uiInit()`.

You are responsible for allocating and initializing this struct. To do so, you simply zero the memory for this struct and set its `Size` field to `sizeof (uiInitError)`. The example in the main section of this page demonstrates how to do this.

In the event of an error, `Message` will contain a NUL-terminated C string in the encoding expected by `fprintf()`. This is in contrast to the rest of libui, which uses UTF-8 strings.
