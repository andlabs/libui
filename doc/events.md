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
typedef void (*uiEventHandler)(uiEvent *event, void *sender, int id, void *args, void *data);
```

TODO do we really include the event and ID?

### `uiNewEvent()`

```c
uiEvent *uiNewEvent(void);
```

`uiNewEvent()` creates a new `uiEvent`.

### `uiEventAddHandler()`

```c
int uiEventAddHandler(uiEvent *e, uiEventHandler handler, void *sender, void *data);
```

### `uiEventDeleteHandler()`

```c
void uiEventDeleteHandle(uiEvent *e, int which);
```

### `uiEventFire()`

```c
void uiEventFire(uiEvent *e, void *sender, void *args);
```

### `uiEventHandlerBlocked()`

```c
TODO_int_or_uiBool uiEventHandlerBlocked(uiEvent *e, int which);
```

### `uiEventSetHandlerBlocked()`

```c
void uiEventSetHandlerBlocked(uiEvent *e, int which, TODO_int_or_uiBool blocked);
```
