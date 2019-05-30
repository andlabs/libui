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

Once you have called `uiInit()`, you should set up the initial UI for your program. Once that is done, you start the program event loop by calling `uiMain()`. `uiMain()` runs the *main event loop*, which processes all events for the GUI and dispatches them to the event handlers you write. As this function does everything for you, all you need to do is call it:

```c
int main(void)
{
	// ...
	uiMain();
	return 0;
}
```

`uiMain()` will not return until one of your event handlers calls `uiQuit()`. `uiQuit()` informs `uiMain()` that it should return, but it won't actually return until the current event handler has finished being processed. Once it does return, you will be dropped back into `main()`.

Event handlers can be more than just actions in response to user action to a control, such as clicking a button. The OS can use events to tell your program things that happen outside your program, such as "the user wants to close your application from within a program list/taskbar/dock". libui provides methods of catching and handling these. You can also schedule code to run when there is no event pending, or on a timed loop. While these scheduled code paths are not technically events per se, they operate exactly like event handlers.

TODO should quit

The simplest way to schedule code to run the next time no event is pending is to use `uiQueueMain()`. `uiQueueMain()` takes a function and some data to pass to that function, and returns immediately, so you can continue working. When `uiMain()` goes to get the next event, it will notice that your function is ready to run, and if there are no events that need to be handled, will do so.

`uiQueueMain()` is special in that it is the only function in libui that you can call from any thread, not just the UI thread. This is intentional: `uiQueueMain()` allows you to communicate between the UI thread and another thread, as all code that is scheduled by `uiQueueMain()` is run on the UI thread. For example, if you are downloading assets in a background thread and want to update a progressbar, you can do so:

```c
void updateProgressBar(void *data)
{
	uiProgressBar *p = uiProgressBar(data);

	uiProgressBarSetValue(p, uiProgressBarValue(p) + 1);
}

extern uiProgressBar *progressbar;

void backgroundThread(void)
{
	while (fileIsDownloading())
		uiQueueMain(updateProgressBar, progressbar);
}
```

If you want to wait for the function in `uiQueueMain()` to be called, you will need to provide some synchronization method yourself. Be careful not to do this if you call `uiQueueMain()` on the UI thread, to avoid deadlocking your program!

TODO timers

## Reference

### `uiInit()`

```c
bool uiInit(void *options, uiInitError *err);
```

`uiInit()` initializes libui. It returns `true` on success and `false` on failure; in the event of a failure, `err` is filled with relevant information explaining the failure.

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

### `uiMain()`

```c
void uiMain(void);
```

`uiMain()` runs the main event loop. It does not return until `uiQuit()` is called.

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
