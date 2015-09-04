// 4 september 2015

typedef struct uiArea uiArea;
typedef struct uiAreaHandler uiAreaHandler;
typedef struct uiAreaDrawParams uiAreaDrawParams;

struct uiAreaHandler {
	// TODO draw
	void (*HScrollConfig)(uiAreaHandler *, uiArea *, uintmax_t *, uintmax_t *);
	void (*VScrollConfig)(uiAreaHandler *, uiArea *, uintmax_t *, uintmax_t *);
};

// TODO uiAreaDrawParams
