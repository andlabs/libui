// 10 february 2017
#include "../ui.h"
#include "uipriv.h"

// TODO this doesn't handle the case where nLines == 0
// TODO this should never happen even if there are no characters??
// TODO figure out how to make this work on GTK+
void uiDrawCaret(uiDrawContext *c, double x, double y, uiDrawTextLayout *layout, size_t pos, int *line)
{
	double xoff;
	uiDrawTextLayoutLineMetrics m;
	struct caretDrawParams cdp;
	uiDrawPath *path;
	uiDrawBrush brush;

	if (*line < 0)
		*line = 0;
	if (*line > (uiDrawTextLayoutNumLines(layout) - 1))
		*line = (uiDrawTextLayoutNumLines(layout) - 1);
	// TODO cap pos
	xoff = uiDrawTextLayoutByteLocationInLine(layout, pos, *line);
	while (xoff < 0) {
		size_t start, end;

		uiDrawTextLayoutLineByteRange(layout, *line, &start, &end);
		if (end < pos)		// too far up
			(*line)++;
		else
			(*line)--;
		xoff = uiDrawTextLayoutByteLocationInLine(layout, pos, *line);
	}
	uiDrawTextLayoutLineGetMetrics(layout, *line, &m);

	caretDrawParams(c, m.Height, &cdp);

	uiDrawSave(c);

	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path,
		// TODO add m.X?
		x + xoff - cdp.xoff, y + m.Y,
		cdp.width, m.Height);
	uiDrawPathEnd(path);
	brush.Type = uiDrawBrushTypeSolid;
	brush.R = cdp.r;
	brush.G = cdp.g;
	brush.B = cdp.b;
	brush.A = cdp.a;
	uiDrawFill(c, path, &brush);
	uiDrawFreePath(path);

	uiDrawRestore(c);
}

void drawTextBackground(uiDrawContext *c, double x, double y, uiDrawTextLayout *layout, size_t start, size_t end, uiDrawBrush *brush, int isSelection)
{
	int line, nLines;
	size_t lstart, lend;
	double layoutwid, layoutht;

	uiDrawTextLayoutExtents(layout, &layoutwid, &layoutht);
	nLines = uiDrawTextLayoutNumLines(layout);
	for (line = 0; line < nLines; line++) {
		uiDrawTextLayoutLineByteRange(layout, line, &lstart, &lend);
		if (start >= lstart && start < lend)
			break;
	}
	while (end - start > 0) {
		uiDrawTextLayoutLineMetrics m;
		double startx, endx;
		uiDrawPath *path;

		uiDrawTextLayoutLineByteRange(layout, line, &lstart, &lend);
		if (lend > end)		// don't cross lines
			lend = end;
		startx = uiDrawTextLayoutByteLocationInLine(layout, start, line);
		// TODO explain this; note the use of start with lend
		endx = layoutwid;
		if (!isSelection || end <= lend)
			endx = uiDrawTextLayoutByteLocationInLine(layout, lend, line);
		uiDrawTextLayoutLineGetMetrics(layout, line, &m);
		path = uiDrawNewPath(uiDrawFillModeWinding);
		uiDrawPathAddRectangle(path,
			x + startx, y + m.Y,
			endx - startx, m.Height);
		uiDrawPathEnd(path);
		uiDrawFill(c, path, brush);
		uiDrawFreePath(path);
		line++;
		start = lend;
	}
}
