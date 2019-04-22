<!-- 12 april 2019 -->

# Initialization and the Main Loop

## Overview

In order to use libui, you must first initialize it. All initialization is done through the `uiInit()` function. This is usually one of the first things you do in your program.

The thread you call `uiInit()` on becomes the "GUI thread". This is the thread that all your UI is done on from the perspective of the operating system, so it is absolutely essential that you do **not** call any other function of libui from any other thread that you create (except for `uiQueueMain()`, which is discussed later).

Furthermore, on some systems, this thread must also be the thread that `main()` is called on; macOS is the notable example here. Therefore, to be safe, you should only call `uiInit()` from `main()`, or from a function that runs on the same thread as `main()`. If you are using a language binding, that binding may provide further instruction on how to do so in that language.

`uiInit()` *can* fail, in which case it will provide information about that failure in a special variable of type `uiInitError` that you provide it. The most useful member of this struct is `Message`, which contains a human-readable error message you can use to diagnose problems initializing libui. If you are using a language binding, the language binding may abstract this away and provide the error to you in a langauge-specific form.

Here is an example of correct use of `uiInit()` in C:

```c
int main(void)
{
	uiInitError err;

	memset(&err, 0, sizeof (uiInitError));
	err.Size = sizeof (uiInitError);
	if (!uiInit(NULL, &err)) {
		fprintf(stderr, "error initializing libui: %s\n", err.Message);
		return 1;
	}
	// ...
}
```

Note that if `uiInit()` fails, you **cannot** use libui's message box functions to report the error.

## Reference

### `uiInit()`

```c
int uiInit(void *options, uiInitError *err);
```

`uiInit()` initializes libui. It returns nonzero on success and zero on failure; in the event of a failure, `err` is filled with relevant information explaining the failure.

`err` is required and must be properly initialized. If `err` is `NULL` or `err->Size` does not match `sizeof (uiError)`, `uiInit()` immediately returns zero without doing anything. If any of the other fields of `err` are not zero-initialized as with `memset(0)`, the behavior is undefined.

`options` must be `NULL`; no options are currently defined. If `options` is not `NULL`, `uiInit()` will return an error without initializing anything.

As part of initialization, the thread that `uiInit()` is called on becomes the GUI thread. All of the functions of libui except `uiQueueMain()` **must** be called from this thread. On platforms where which thread you run GUI code on is restricted, `uiMain()` will return an error if it is called from the wrong thread; to avoid this, you should always call `uiInit()` from the thread that `main()` itself is called from.

If `uiInit()` fails, no other libui function is safe to call. This means that you cannot use any of libui's message box functions to report the error.

**Notes for language binding authors**: Your language will likely provide its own preferred mechanism for reporting errors. You should wrap `uiInit()` to return errors this way, creating and managing the memory for `uiInitError` yourself and transforming the returned error according to both the format of `uiInitError` described below and the rules for encoding errors in your language of choice.

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
