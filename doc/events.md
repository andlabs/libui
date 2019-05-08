<!-- 7 may 2019 -->

# Event Handling

## Overview

TODO erase this and start over {
When the user tries to interacts with your program, your program needs a way to be notified of that interaction. This is typically done by sending an *event* to the program that describes what interaction takes place and how. In turn, the operating system can interpret general events (such as "the user has pressed the primary mouse button") and send more specialized ones (such as "the user clicked on this specific checkbox on the screen"). This paragraph is super awkward and I'm not sure what to do about it, because I was about to talk about how events are *queued* and that `uiMain()` reads from that queue in `uiMain()` and since that function does not return through the lifetime of your program you'll need to use callbacks but I feel like a primary school teacher saying things this way.

All events in libui are dispatched through `uiEvent` objects, and you opt into handling an event by adding a handler to a `uiEvent`. Handlers are functions with the following signature:
}

## Reference

### `uiEvent`

```c
typedef struct uiEvent uiEvent;
```

`uiEvent` is an opaque type that describes an event.

### `uiEventHandler`

```c
typedef void (*uiEventHandler)(uiEvent *event, void *sender, void *args, void *data);
```

`uiEventHandler` is a pointer to the type of function that all event hanlder functions must take:

```c
void eventHandler(uiEvent *event, void *sender, void *args, void *data);
```

`event` is the event that was fired to cuase this function to be called. `sender` is the object that fired this event; this should typically be the object that the event happened to. `args` is data specific to the specific invocation of the event handler that is currently active, if any; consult the documentation for the event you are interested in for info on how to interpret `args`. `data` is the data you specified when registering the given function as an event handler; it is the same for all calls to an event.

### `uiNewEvent()`

```c
uiEvent *uiNewEvent(void);
```

`uiNewEvent()` creates a new `uiEvent`.

### `uiEventAddHandler()`

```c
int uiEventAddHandler(uiEvent *event, uiEventHandler handler, void *sender, void *data);
```

### `uiEventFire()`

```c
void uiEventFire(uiEvent *event, void *sender, void *args);
```
