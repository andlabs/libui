<!-- 12 april 2019 -->

# Initialization and the Main Loop

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
