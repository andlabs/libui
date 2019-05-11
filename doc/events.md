<!-- 7 may 2019 -->

# Event Handling

## Overview

## Reference

### `uiEvent`

```c
typedef struct uiEvent uiEvent;
```

`uiEvent` is an opaque type that describes an event.

### `uiEventHandler`

```c
typedef void (*uiEventHandler)(void *sender, void *args, void *data);
```

`uiEventHandler` is the type of a pointer to an event handler function registered with `uiEventAddHandler()`.

### `uiEventOptions`

```c
typedef struct uiEventOptions uiEventOptions;
struct uiEventOptions {
	// TODO size
	TODO_int_or_uiBool Global;
};
```

`uiEventOptions` describes the properties of a `uiEvent`; you pass these to `uiNewEvent()`.

If `Global` is TODO_nonzero_or_uiTrue, the event is "global" — there are no specific senders, and all registered handler functions are called when the event is fired. Otherwise, the event has specific senders, and only handlers registered with the sender that is currently firing the event will be called.

### `uiNewEvent()`

```c
uiEvent *uiNewEvent(uiEventOptions *options);
```

`uiNewEvent()` creates a new `uiEvent`.

### `uiEventAddHandler()`

```c
int uiEventAddHandler(uiEvent *e, uiEventHandler handler, void *sender, void *data);
```

`uiEventAddHandler()` adds a handler to an event. When the event is fired, `handler` is called with `data` as its last parameter.

For a global event handler, `sender` must be `NULL`. For a non-global event handler, `sender` must be a non-`NULL` object that may fire the given event, and the handler will only be called if `sender` actually does fire the event.

The return value is an identifier that may be used to delete or block the event. This identifier is not passed directly to the handler function; if you would like the handler function to know the identifier, store it somewhere in `data`.

Note that event handlers are NOT deduplicated; if you call `uiEventAddHandler()` twice with the same `handler`, then `handler` will be registered twice and will thus be called twice, even if `sender` and/or `data` are the same, and `uiEventAddHandler()` will return two distinct identifiers.

### `uiEventDeleteHandler()`

```c
void uiEventDeleteHandler(uiEvent *e, int which);
```

### `uiEventFire()`

```c
void uiEventFire(uiEvent *e, void *sender, void *args);
```

`uiEventFire()` fires the given event, calling associated handlers sequentially and returning only when the last handler returns. If no associated handlers are registered, `uiEventFire()` does nothing.

If the event is a global event, `sender` must be `NULL`, and all registered handlers will be called. Otherwise, `sender` must be non-`NULL`, and only handlers that were registered with `uiEventAddHandler()` with the same `sender` will be called. Any handlers that are blocked by a call to `uiEventSetHandlerBlocked()` will not be called.

Each handler that is to be called will receive whatever you pass in as `args` to its respective `args` argument; this allows `args` to be used to send information about the current firing of the event to the handlers. This argument can be `NULL` if no such information is available.

Note that the order that handler functions are called in is unspecified.

### `uiEventHandlerBlocked()`

```c
TODO_int_or_uiBool uiEventHandlerBlocked(uiEvent *e, int which);
```

`uiEventHandlerBlocked()` returns whether or not the given registered event handler is *blocked*. A blocked event handler will not be called by `uiEventFire()`, even if that handler matches the parameters passed to `uiEventFire()`. `which` should be the identifier of a previously registered event handler as returned by `uiEventAddHandler()`.

### `uiEventSetHandlerBlocked()`

```c
void uiEventSetHandlerBlocked(uiEvent *e, int which, TODO_int_or_uiBool blocked);
```

`uiEventSetHandlerBlocked()` changes whether or not the given registered event handler is bocked. `which` should be the identifier of a previously registered event handler as returned by `uiEventAddHandler()`.

The effect of calling this function on a handler that matches a currently active `uiEventFire()` is unspecified.
