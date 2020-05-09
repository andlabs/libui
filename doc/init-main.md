<!-- 12 april 2019 -->

# Initialization and the Main Loop

## Overview

TODO

## Reference

### `uiInit()`

```c
bool uiInit(void *options, uiInitError *err);
```

`uiInit()` initializes libui. It returns `true` on success and `false` on failure; in the event of a failure, `err` is filled with relevant information explaining the failure.

`err` is required and must be properly initialized. If `err` is `NULL` or `err->Size` does not match `sizeof (uiError)`, `uiInit()` immediately returns `false` without doing anything. If any of the other fields of `err` are not zero-initialized as with `memset(0)`, the behavior is undefined.

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

### `uiMain()`

```c
void uiMain(void);
```

`uiMain()` runs the main event loop. It does not return until `uiQuit()` is called.

TODO make it illegal to call more than once?

### `uiQuit()`

```c
void uiQuit(void);
```

`uiQuit()` causes `uiMain()` to return once the current event has finished processing.

TODO safety of calling `uiMain()` again after `uiQuit()`?

TODO behavior of calling `uiQuit()` outside of a `uiMain()` (before, after, etc.)

### `uiQueueMain()`

```c
void uiQueueMain(void (*f)(void *data), void *data);
```

`uiQueueMain()` schedules `f` to be called with `data` as a parameter on the next iteration of the main loop when no event is pending. It returns immediately.

`uiQueueMain()` can safely be called from any thread, not just the UI thread.

If you call `uiQueueMain()` in sequence, the functions will execute in that order; if other threads are calling `uiQueueMain()` at the same time, there is no guarantee as to whether the calls are interleaved, but the order per-thread will be maintained.
