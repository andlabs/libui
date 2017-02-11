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
