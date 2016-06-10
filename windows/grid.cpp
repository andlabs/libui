// 10 june 2016
#include "uipriv_windows.hpp"

// TODO compare with GTK+:
// - what happens if you call InsertAt() twice?
// - what happens if you call Append() twice?

// TODOs
// - make ALL the controls handle hidden children right

struct gridChild {
	uiControl *c;
	intmax_t left;
	intmax_t top;
	intmax_t xspan;
	intmax_t yspan;
	int hexpand;
	uiAlign halign;
	int vexpand;
	uiAlign valign;

	// have these here so they don't need to be reallocated each relayout
	intmax_t finalx, finaly;
	intmax_t finalwidth, finalheight;
	intmax_t minwidth, minheight;
};

struct uiGrid {
	uiWindowsControl c;
	HWND hwnd;
	std::vector<struct gridChild *> *children;
	std::map<uiControl *, size_t> *indexof;
	int padded;

	intmax_t xmin, ymin;
	intmax_t xmax, ymax;
};

#define xcount(g) ((g)->xmax - (g)->xmin)
#define ycount(g) ((g)->ymax - (g)->ymin)
#define toxindex(g, x) ((x) - (g)->xmin)
#define toyindex(g, y) ((y) - (g)->ymin)

class gridLayoutData {
	size_t ycount;
public:
	intmax_t **gg;		// topological map gg[y][x] = control index
	intmax_t *colwidths;
	intmax_t *rowheights;
	bool *hexpand;
	bool *vexpand;

	gridLayoutData(uiGrid *g)
	{
		size_t i;
		intmax_t x, y;

		this->gg = new intmax_t *[ycount(g)];
		for (y = 0; y < ycount(g); y++) {
			this->gg[y] = new intmax_t[xcount(g)];
			for (x = 0; x < xcount(g); x++)
				this->gg[y][x] = -1;
		}

		for (i = 0; i < g->children->size(); i++) {
			struct gridChild *gc;

			gc = (*(g->children))[i];
			for (y = gc->top; y < gc->top + gc->yspan; y++)
				for (x = gc->left; x < gc->left + gc->xspan; x++)
					this->gg[toyindex(g, y)][toxindex(g, x)] = i;
		}

		this->colwidths = new intmax_t[xcount(g)];
		ZeroMemory(this->colwidths, xcount(g) * sizeof (intmax_t));
		this->rowheights = new intmax_t[ycount(g)];
		ZeroMemory(this->rowheights, ycount(g) * sizeof (intmax_t));
		this->hexpand = new bool[xcount(g)];
		ZeroMemory(this->hexpand, xcount(g) * sizeof (bool));
		this->vexpand = new bool[ycount(g)];
		ZeroMemory(this->vexpand, ycount(g) * sizeof (bool));

		this->ycount = ycount(g);
	}

	~gridLayoutData()
	{
		size_t y;

		delete[] this->hexpand;
		delete[] this->vexpand;
		delete[] this->colwidths;
		delete[] this->rowheights;
		for (y = 0; y < this->ycount; y++)
			delete[] this->gg[y];
		delete[] this->gg;
	}
};

static void gridPadding(uiGrid *g, int *xpadding, int *ypadding)
{
	uiWindowsSizing sizing;

	*xpadding = 0;
	*ypadding = 0;
	if (g->padded) {
		uiWindowsGetSizing(g->hwnd, &sizing);
		uiWindowsSizingStandardPadding(&sizing, xpadding, ypadding);
	}
}

static void gridRelayout(uiGrid *g)
{
	RECT r;
	intmax_t x, y, width, height;
	gridLayoutData *ld;
	int xpadding, ypadding;
	intmax_t ix, iy;
	intmax_t iwidth, iheight;
	int i;
	struct gridChild *gc;
	intmax_t nhexpand, nvexpand;

	if (g->children->size() == 0)
		return;		// nothing to do

	uiWindowsEnsureGetClientRect(g->hwnd, &r);
	x = r.left;
	y = r.top;
	width = r.right - r.left;
	height = r.bottom - r.top;

	gridPadding(g, &xpadding, &ypadding);
	ld = new gridLayoutData(g);

	// 0) discount padding from width/height
	width -= (xcount(g) - 1) * xpadding;
	height -= (ycount(g) - 1) * ypadding;

	// 1) compute colwidths and rowheights before handling expansion
	// we only count non-spanning controls to avoid weirdness
	for (iy = 0; iy < ycount(g); iy++)
		for (ix = 0; ix < xcount(g); ix++) {
			i = ld->gg[iy][ix];
			if (i == -1)
				continue;
			gc = (*(g->children))[i];
			uiWindowsControlMinimumSize(uiWindowsControl(gc->c), &iwidth, &iheight);
			if (gc->xspan == 1)
				if (ld->colwidths[ix] < iwidth)
					ld->colwidths[ix] = iwidth;
			if (gc->yspan == 1)
				if (ld->rowheights[iy] < iheight)
					ld->rowheights[iy] = iheight;
			// save these for step 6
			gc->minwidth = iwidth;
			gc->minheight = iheight;
		}

	// 2) figure out which rows/columns expand but not span
	// we need to know which expanding rows/columns don't span before we can handle the ones that do
	for (i = 0; i < g->children->size(); i++) {
		gc = (*(g->children))[i];
		if (gc->hexpand && gc->xspan == 1)
			ld->hexpand[gc->left] = true;
		if (gc->vexpand && gc->yspan == 1)
			ld->vexpand[gc->top] = true;
	}

	// 3) figure out which rows/columns expand that do span
	// the way we handle this is simple: if none of the spanned rows/columns expand, make all rows/columns expand
	for (i = 0; i < g->children->size(); i++) {
		gc = (*(g->children))[i];
		if (gc->hexpand && gc->xspan != 1) {
			bool doit = true;

			for (ix = gc->left; ix < gc->left + gc->xspan; ix++)
				if (ld->hexpand[ix]) {
					doit = false;
					break;
				}
			if (doit)
				for (ix = gc->left; ix < gc->left + gc->xspan; ix++)
					ld->hexpand[ix] = true;
		}
		if (gc->vexpand && gc->yspan != 1) {
			bool doit = true;

			for (iy = gc->top; iy < gc->top + gc->yspan; iy++)
				if (ld->vexpand[iy]) {
					doit = false;
					break;
				}
			if (doit)
				for (iy = gc->top; iy < gc->top + gc->yspan; iy++)
					ld->vexpand[iy] = true;
		}
	}


	// 4) compute and assign expanded widths/heights
	nhexpand = 0;
	nvexpand = 0;
	for (i = 0; i < xcount(g); i++)
		if (ld->hexpand[i])
			nhexpand++;
		else
			width -= ld->colwidths[i];
	for (i = 0; i < ycount(g); i++)
		if (ld->vexpand[i])
			nvexpand++;
		else
			height -= ld->rowheights[i];
	for (i = 0; i < xcount(g); i++)
		if (ld->hexpand[i])
			ld->colwidths[i] = width / nhexpand;
	for (i = 0; i < ycount(g); i++)
		if (ld->vexpand[i])
			ld->rowheights[i] = height / nvexpand;

	// 5) reset the final coordinates for the next step
	for (i = 0; i < g->children->size(); i++) {
		gc = (*(g->children))[i];
		gc->finalx = 0;
		gc->finaly = 0;
		gc->finalwidth = 0;
		gc->finalheight = 0;
	}

	// 6) compute cell positions and sizes
	for (iy = 0; iy < ycount(g); y++) {
		intmax_t curx;
		int prev;

		curx = 0;
		prev = -1;
		for (ix = 0; ix < xcount(g); ix++) {
			i = ld->gg[iy][ix];
			if (i != -1) {
				gc = (*(g->children))[i];
				if (iy == gc->top) {			// don't repeat this step if the control spans vertically
					if (i != prev)
						gc->finalx = curx;
					else
						gc->finalwidth += xpadding;
					gc->finalwidth += ld->colwidths[ix];
				}
			}
			curx += ld->colwidths[ix] + xpadding;
			prev = i;
		}
	}
	for (ix = 0; ix < xcount(g); ix++) {
		intmax_t cury;
		int prev;

		cury = 0;
		prev = -1;
		for (iy = 0; iy < ycount(g); iy++) {
			i = ld->gg[iy][ix];
			if (i != -1) {
				gc = (*(g->children))[i];
				if (x == gc->top) {		// don't repeat this step if the control spans horizontally
					if (i != prev)
						gc->finaly = cury;
					else
						gc->finalheight += ypadding;
					gc->finalheight += ld->rowheights[iy];
				}
			}
			cury += ld->rowheights[iy] + ypadding;
			prev = i;
		}
	}

	// 7) everything as it stands now is set for xalign == Fill yalign == Fill; set the correct alignments
	// this is why we saved minwidth/minheight above
	for (i = 0; i < g->children->size(); i++) {
		gc = (*(g->children))[i];
		if (gc->halign != uiAlignFill) {
			switch (gc->halign) {
			case uiAlignEnd:
				gc->finalx += gc->finalwidth - gc->minwidth;
				break;
			case uiAlignCenter:
				gc->finalx += (gc->finalwidth - gc->minwidth) / 2;
				break;
			}
			gc->finalwidth = gc->minwidth;		// for all three
		}
		if (gc->valign != uiAlignFill) {
			switch (gc->valign) {
			case uiAlignEnd:
				gc->finaly += gc->finalheight - gc->minheight;
				break;
			case uiAlignCenter:
				gc->finaly += (gc->finalheight - gc->minheight) / 2;
				break;
			}
			gc->finalheight = gc->minheight;		// for all three
		}
	}

	// 8) and FINALLY we resize
	for (iy = 0; iy < ycount(g); iy++)
		for (ix = 0; ix < xcount(g); ix++) {
			i = ld->gg[iy][ix];
			if (i != -1) {		// treat empty cells like spaces
				gc = (*(g->children))[i];
				uiWindowsEnsureMoveWindowDuringResize(
					(HWND) uiControlHandle(gc->c),
 					gc->finalx,//TODO + x,
					gc->finaly,//TODO + y,
					gc->finalwidth,
					gc->finalheight);
			}
		}

	delete ld;
}

static void uiGridDestroy(uiControl *c)
{
	uiGrid *g = uiGrid(c);

	for (struct gridChild *gc : *(g->children)) {
		uiControlSetParent(gc->c, NULL);
		uiControlDestroy(gc->c);
		uiFree(gc);
	}
	delete g->indexof;
	delete g->children;
	uiWindowsEnsureDestroyWindow(g->hwnd);
	uiFreeControl(uiControl(g));
}

uiWindowsControlDefaultHandle(uiGrid)
uiWindowsControlDefaultParent(uiGrid)
uiWindowsControlDefaultSetParent(uiGrid)
uiWindowsControlDefaultToplevel(uiGrid)
uiWindowsControlDefaultVisible(uiGrid)
uiWindowsControlDefaultShow(uiGrid)
uiWindowsControlDefaultHide(uiGrid)
uiWindowsControlDefaultEnabled(uiGrid)
uiWindowsControlDefaultEnable(uiGrid)
uiWindowsControlDefaultDisable(uiGrid)

static void uiGridSyncEnableState(uiWindowsControl *c, int enabled)
{
	uiGrid *g = uiGrid(c);

	if (uiWindowsShouldStopSyncEnableState(uiWindowsControl(g), enabled))
		return;
	for (const struct gridChild *gc : *(g->children))
		uiWindowsControlSyncEnableState(uiWindowsControl(gc->c), enabled);
}

uiWindowsControlDefaultSetParentHWND(uiGrid)

static void uiGridMinimumSize(uiWindowsControl *c, intmax_t *width, intmax_t *height)
{
	uiGrid *g = uiGrid(c);
	int xpadding, ypadding;
	gridLayoutData *ld;
	intmax_t x, y;
	int i;
	struct gridChild *gc;
	intmax_t minwid, minht;
	intmax_t colwidth, rowheight;

	*width = 0;
	*height = 0;
	if (g->children->size() == 0)
		return;		// nothing to do

	gridPadding(g, &xpadding, &ypadding);
	ld = new gridLayoutData(g);

	// 1) compute colwidths and rowheights before handling expansion
	// TODO put this in its own function (but careful about the spanning calculation in gridRelayout())
	for (y = 0; y < ycount(g); y++)
		for (x = 0; x < xcount(g); x++) {
			i = ld->gg[y][x];
			if (i == -1)
				continue;
			gc = (*(g->children))[i];
			uiWindowsControlMinimumSize(uiWindowsControl(gc->c), &minwid, &minht);
			// allot equal space in the presence of spanning to keep things sane
			if (ld->colwidths[x] < minwid / gc->xspan)
				ld->colwidths[x] = minwid / gc->xspan;
			if (ld->rowheights[y] < minht / gc->yspan)
				ld->rowheights[y] = minht / gc->yspan;
			// save these for step 6
			gc->minwidth = minwid;
			gc->minheight = minht;
		}

	// 2) compute total column width/row height
	colwidth = 0;
	rowheight = 0;
	for (x = 0; x < xcount(g); x++)
		colwidth += ld->colwidths[x];
	for (y = 0; y < ycount(g); y++)
		rowheight += ld->rowheights[y];

	// and that's it; just account for padding
	*width = colwidth + (g->xmax-1) * xpadding;
	*height = rowheight + (g->ymax-1) * ypadding;
}

static void uiGridMinimumSizeChanged(uiWindowsControl *c)
{
	uiGrid *g = uiGrid(c);

	if (uiWindowsControlTooSmall(uiWindowsControl(g))) {
		uiWindowsControlContinueMinimumSizeChanged(uiWindowsControl(g));
		return;
	}
	gridRelayout(g);
}

uiWindowsControlDefaultLayoutRect(uiGrid)
uiWindowsControlDefaultAssignControlIDZOrder(uiGrid)

// must have called gridRecomputeMinMax() first
static void gridArrangeChildren(uiGrid *g)
{
	LONG_PTR controlID;
	HWND insertAfter;
	gridLayoutData *ld;
	bool *visited;
	intmax_t x, y;
	int i;
	struct gridChild *gc;

	if (g->children->size() == 0)
		return;		// nothing to do
	ld = new gridLayoutData(g);
	controlID = 100;
	insertAfter = NULL;
	visited = new bool[g->children->size()];
	ZeroMemory(visited, g->children->size() * sizeof (bool));
	for (y = 0; y < ycount(g); y++)
		for (x = 0; x < xcount(g); x++) {
			i = ld->gg[y][x];
			if (i == -1)
				continue;
			if (visited[i])
				continue;
			visited[i] = true;
			gc = (*(g->children))[i];
			uiWindowsControlAssignControlIDZOrder(uiWindowsControl(gc->c), &controlID, &insertAfter);
		}
	delete[] visited;
	delete ld;
}

static void gridRecomputeMinMax(uiGrid *g)
{
	bool first = true;

	for (struct gridChild *gc : *(g->children)) {
		if (first) {
			g->xmin = gc->left;
			g->ymin = gc->top;
			g->xmax = gc->left + gc->xspan;
			g->ymax = gc->top + gc->yspan;
			first = false;
			continue;
		}
		if (g->xmin > gc->left)
			g->xmin = gc->left;
		if (g->ymin > gc->top)
			g->ymin = gc->top;
		if (g->xmax < (gc->left + gc->xspan))
			g->xmax = gc->left + gc->xspan;
		if (g->ymax < (gc->top + gc->yspan))
			g->ymax = gc->top + gc->yspan;
	}
}

static struct gridChild *toChild(uiControl *c, intmax_t xspan, intmax_t yspan, int hexpand, uiAlign halign, int vexpand, uiAlign valign)
{
	struct gridChild *gc;

	if (xspan < 0)
		userbug("You cannot have a negative xspan in a uiGrid cell.");
	if (yspan < 0)
		userbug("You cannot have a negative yspan in a uiGrid cell.");
	gc = uiNew(struct gridChild);
	gc->c = c;
	gc->xspan = xspan;
	gc->yspan = yspan;
	gc->hexpand = hexpand;
	gc->halign = halign;
	gc->vexpand = vexpand;
	gc->valign = valign;
	return gc;
}

static void add(uiGrid *g, struct gridChild *gc)
{
	uiControlSetParent(gc->c, uiControl(g));
	uiWindowsControlSetParentHWND(uiWindowsControl(gc->c), g->hwnd);
	g->children->push_back(gc);
	(*(g->indexof))[gc->c] = g->children->size() - 1;
	gridRecomputeMinMax(g);
	gridArrangeChildren(g);
	uiWindowsControlMinimumSizeChanged(uiWindowsControl(g));
}

void uiGridAppend(uiGrid *g, uiControl *c, intmax_t left, intmax_t top, intmax_t xspan, intmax_t yspan, int hexpand, uiAlign halign, int vexpand, uiAlign valign)
{
	struct gridChild *gc;

	gc = toChild(c, xspan, yspan, hexpand, halign, vexpand, valign);
	gc->left = left;
	gc->top = top;
	add(g, gc);
}

// TODO decide what happens if existing is NULL
void uiGridInsertAt(uiGrid *g, uiControl *c, uiControl *existing, uiAt at, intmax_t xspan, intmax_t yspan, int hexpand, uiAlign halign, int vexpand, uiAlign valign)
{
	struct gridChild *gc;
	struct gridChild *other;

	gc = toChild(c, xspan, yspan, hexpand, halign, vexpand, valign);
	other = (*(g->children))[(*(g->indexof))[existing]];
	switch (at) {
	case uiAtLeading:
		gc->left = other->left - gc->xspan;
		gc->top = other->top;
		break;
	case uiAtTop:
		gc->left = other->left;
		gc->top = other->top - gc->yspan;
		break;
	case uiAtTrailing:
		gc->left = other->left + other->xspan;
		gc->top = other->top;
		break;
	case uiAtBottom:
		gc->left = other->left;
		gc->top = other->top + other->yspan;
		break;
	// TODO add error checks to ALL enums
	}
	add(g, gc);
}

int uiGridPadded(uiGrid *g)
{
	return g->padded;
}

void uiGridSetPadded(uiGrid *g, int padded)
{
	g->padded = padded;
	uiWindowsControlMinimumSizeChanged(uiWindowsControl(g));
}

static void onResize(uiWindowsControl *c)
{
	gridRelayout(uiGrid(c));
}

uiGrid *uiNewGrid(void)
{
	uiGrid *g;

	uiWindowsNewControl(uiGrid, g);

	g->hwnd = uiWindowsMakeContainer(uiWindowsControl(g), onResize);

	g->children = new std::vector<struct gridChild *>;
	g->indexof = new std::map<uiControl *, size_t>;

	return g;
}
