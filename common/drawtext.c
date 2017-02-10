// 10 february 2017
#include "../ui.h"
#include "uipriv.h"

void uiDrawCaret(uiDrawContext *c, double x, double y, uiDrawTextLayout *layout, size_t pos, int *line)
{
	double xoff;
	uiDrawTextLayoutLineMetrics m;
	struct caretDrawParams cdp;
	uiDrawPath *path;
	uiDrawBrush brush;

	xoff = uiDrawTextLayoutByteLocationInLine(layout, pos, *line);
	if (xoff < 0) {
		size_t start, end;
		int incr;

		if (*line > (uiDrawTextLayoutNumLines(layout) - 1)) {
			*line = (uiDrawTextLayoutNumLines(layout) - 1);
			incr = -1;
		} else {
			uiDrawTextLayoutLineByteRange(layout, *line, &start, &end);
			incr = 1;
			if (end < pos)
				incr = -1;
		}
		while (xoff < 0) {
			*line += incr;
			xoff = uiDrawTextLayoutByteLocationInLine(layout, pos, *line);
		}
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
