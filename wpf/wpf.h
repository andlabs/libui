// 24 november 2015

#ifndef EXPORT
#define EXPORT __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wpfWindow wpfWindow;

extern EXPORT wpfWindow *wpfNewWindow(const char *title, int width, int height);
extern EXPORT void wpfWindowOnClosing(wpfWindow *w, void (*f)(wpfWindow *w, void *data), void *data);

extern EXPORT void wpfInit(void);
extern EXPORT void wpfRun(void);
extern EXPORT void wpfQuit(void);

#ifdef __cplusplus
}
#endif
