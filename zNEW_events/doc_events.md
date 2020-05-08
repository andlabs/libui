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

`args` will be the respective argument passed to `uiEventFire()`, and `sender` and `data` will be the respective arguments passed to `uiEventAddHandler()`.

### `uiEventOptions`

```c
typedef struct uiEventOptions uiEventOptions;
struct uiEventOptions {
	size_t Size;
	bool Global;
};
```

`uiEventOptions` describes the properties of a `uiEvent`; you pass these to `uiNewEvent()`.

You are responsible for allocating and initializing this struct. To do so, you simply zero the memory for this struct and set its `Size` field to `sizeof (uiEventOptions)`. (TODO put this in a common place)

If `Global` is true, the event is "global" — there are no specific senders, and all registered handler functions are called when the event is fired. Otherwise, the event has specific senders, and only handlers registered with the sender that is currently firing the event will be called.

### `uiNewEvent()`

```c
uiEvent *uiNewEvent(const uiEventOptions *options);
```

`uiNewEvent()` creates a new `uiEvent` with the given options.

It is a programmer error to specify `NULL` for `options`.

### `uiEventFree()`

```c
uiprivExtern void uiEventFree(uiEvent *e);
```

Frees the given event. The event must not be a libui-provided event, and must not have any handlers registered to it.

It is a programmer error to pass `NULL` for `e`. It is also a programmer error to call `uiEventFree()` on an event while that event is being fired.

### `uiEventAddHandler()`

```c
int uiEventAddHandler(uiEvent *e, uiEventHandler handler, void *sender, void *data);
```

`uiEventAddHandler()` adds a handler to an event. When the event is fired, `handler` is called with `data` as its last parameter.

For a global event handler, `sender` must be `NULL`. For a non-global event handler, `sender` must be a non-`NULL` object that may fire the given event, and the handler will only be called if `sender` actually does fire the event.

The return value is an identifier that may be used to delete or block the event. This identifier is not passed directly to the handler function; if you would like the handler function to know the identifier, store it somewhere in `data`.

Note that event handlers are NOT deduplicated; if you call `uiEventAddHandler()` twice with the same `handler`, then `handler` will be registered twice and will thus be called twice, even if `sender` and/or `data` are the same, and `uiEventAddHandler()` will return two distinct identifiers.

It is a programmer error to specify `NULL` for `e` or `handler`. It is also a programmer error to call `uiEventAddHandler()` on an event while that event is being fired.

### `uiEventDeleteHandler()`

```c
void uiEventDeleteHandler(uiEvent *e, int id);
```

`uiEventDeleteHandler()` removes an event handler registration; you specify which handler to unregister by passing in the ID returned from `uiEventAddHandler()`.

It is a programmer error to specify `NULL` for `e` or a currently unregistered value for `id`. It is also a programmer error to call `uiEventDeleteHandler()` on an event while that event is being fired.

### `uiEventFire()`

```c
void uiEventFire(uiEvent *e, void *sender, void *args);
```

`uiEventFire()` fires the given event, calling associated handlers sequentially and returning only when the last handler returns. If no associated handlers are registered, `uiEventFire()` does nothing.

If the event is a global event, `sender` must be `NULL`, and all registered handlers will be called. Otherwise, `sender` must be non-`NULL`, and only handlers that were registered with `uiEventAddHandler()` with the same `sender` will be called. Any handlers that are blocked by a call to `uiEventSetHandlerBlocked()` will not be called.

Each handler that is to be called will receive whatever you pass in as `args` to its respective `args` argument; this allows `args` to be used to send information about the current firing of the event to the handlers. This argument can be `NULL` if no such information is available.

Note that the order that handler functions are called in is unspecified.

It is a programmer error to specify `NULL` for `e`. It is also a programmer error to fire an event again while it is already being fired.

### `uiEventHandlerBlocked()`

```c
bool uiEventHandlerBlocked(const uiEvent *e, int id);
```

`uiEventHandlerBlocked()` returns whether or not the given registered event handler is *blocked*. A blocked event handler will not be called by `uiEventFire()`, even if that handler matches the parameters passed to `uiEventFire()`. `id` should be the identifier of a previously registered event handler as returned by `uiEventAddHandler()`.

It is a programmer error to specify `NULL` for `e` or a currently unregistered event identifier for `id`.

### `uiEventSetHandlerBlocked()`

```c
void uiEventSetHandlerBlocked(uiEvent *e, int id, bool blocked);
```

`uiEventSetHandlerBlocked()` changes whether or not the given registered event handler is bocked. `id` should be the identifier of a previously registered event handler as returned by `uiEventAddHandler()`.

It is a programmer error to specify `NULL` for `e` or a currently unregistered event identifier for `id`. It is also a programmer error to call `uiEventSetHandlerBlocked()` on an event while that event is being fired.

## `uiEventInvalidateSender()`

```c
void uiEventInvalidateSender(uiEvent *e, void *sender);
```

`uiEventInvalidateSender()` instructs any registered handlers of `e` with sender `sender` that they should no longer fire. The handlers are not deleted (since the handler IDs may be reused, which may lead to bugs in client code), nor are they blocked with `uiEventHandlerSetBlocked()` (so the value of the blocked flag will be unaffected), but they will no longer actually run when the event is fired with the given sender pointer. Any future handlers registered with the same sender pointer will NOT be deactivated thus; only the existing ones are. The affected handlers can safely be deleted or blocked (the blocked flag merely no longer has any effect).

The idea behind this function is that it is to be called when `sender` is destroyed, so `sender` does not need to worry about whether it has an event handler registered against it or not. Since it is possible for two objects whose lifetimes never overlap to have the same pointer value, simply blacklisting the sender pointer is insufficient.

It is a programmer error to specify `NULL` or a global event for `e`, or to specify `NULL` for `sender`. It is also a programmer error to call `uiEventInvalidateSender()` on an event while that event is being fired.
