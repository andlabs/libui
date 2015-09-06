// 4 september 2015

typedef struct uiArea uiArea;
typedef struct uiAreaHandler uiAreaHandler;
typedef struct uiAreaDrawParams uiAreaDrawParams;

struct uiAreaHandler {
	// TODO draw
	uintmax_t (*HScrollMax)(uiAreaHandler *, uiArea *);
	uintmax_t (*VScrollMax)(uiAreaHandler *, uiArea *);
};

// TODO uiAreaDrawParams
