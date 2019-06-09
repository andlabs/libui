// 6 april 2015
// note: this file should not include ui.h, as the OS-specific ui_*.h files are included between that one and this one in the OS-specific uipriv_*.h* files
#include <stdarg.h>
#include <string.h>
#include "controlsigs.h"
#include "utf.h"

#ifdef __cplusplus
extern "C" {
#endif

// OS-specific init.* or main.* files
extern uiInitOptions uiprivOptions;

// shouldquit.c
extern int uiprivShouldQuit(void);

// areaevents.c
typedef struct uiprivClickCounter uiprivClickCounter;
// you should call Reset() to zero-initialize a new instance
// it doesn't matter that all the non-count fields are zero: the first click will fail the curButton test straightaway, so it'll return 1 and set the rest of the structure accordingly
struct uiprivClickCounter {
	int curButton;
	int rectX0;
	int rectY0;
	int rectX1;
	int rectY1;
	uintptr_t prevTime;
	int count;
};
extern int uiprivClickCounterClick(uiprivClickCounter *c, int button, int x, int y, uintptr_t time, uintptr_t maxTime, int32_t xdist, int32_t ydist);
extern void uiprivClickCounterReset(uiprivClickCounter *);
extern int uiprivFromScancode(uintptr_t, uiAreaKeyEvent *);

// matrix.c
extern void uiprivFallbackSkew(uiDrawMatrix *, double, double, double, double);
extern void uiprivScaleCenter(double, double, double *, double *);
extern void uiprivFallbackTransformSize(uiDrawMatrix *, double *, double *);

// OS-specific text.* files
extern int uiprivStricmp(const char *a, const char *b);

#ifdef __cplusplus
}
#endif
