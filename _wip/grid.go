// 31 august 2014

package ui

import (
	"fmt"
)

// Grid is a Control that arranges other Controls in a grid.
// Grid is a very powerful container: it can position and size each Control in several ways and can (and must) have Controls added to it at any time, in any direction.
// it can also have Controls spanning multiple rows and columns.
//
// Each Control in a Grid has associated "expansion" and "alignment" values in both the X and Y direction.
// Expansion determines whether all cells in the same row/column are given whatever space is left over after figuring out how big the rest of the Grid should be.
// Alignment determines the position of a Control relative to its cell after computing the above.
// The special alignment Fill can be used to grow a Control to fit its cell.
// Note that expansion and alignment are independent variables.
// For more information on expansion and alignment, read https://developer.gnome.org/gtk3/unstable/ch28s02.html.
type Grid interface {
	Control

	// Add adds a Control to the Grid.
	// If this is the first Control in the Grid, it is merely added; nextTo should be nil.
	// Otherwise, it is placed relative to nextTo.
	// If nextTo is nil, it is placed next to the previously added Control.
	// The effect of adding the same Control multiple times is undefined, as is the effect of adding a Control next to one not present in the Grid.
	// The effect of overlapping spanning Controls is also undefined.
	// Add panics if either xspan or yspan are zero or negative.
	Add(control Control, nextTo Control, side Side, xexpand bool, xalign Align, yexpand bool, yalign Align, xspan int, yspan int)

	// Padded and SetPadded get and set whether the controls of the Grid have padding between them.
	// The size of the padding is platform-dependent.
	Padded() bool
	SetPadded(padded bool)
}

// Align represents the alignment of a Control in its cell of a Grid.
type Align uint

const (
	LeftTop Align = iota
	Center
	RightBottom
	Fill
)

// Side represents a side of a Control to add other Controls to a Grid to.
type Side uint

const (
	West Side = iota
	East
	North
	South
	nSides
)

type grid struct {
	controls []gridCell
	indexof  map[Control]int
	prev     int
	parent	*controlParent
	padded	bool

	xmax int
	ymax int
}

type gridCell struct {
	control Control
	xexpand bool
	xalign  Align
	yexpand bool
	yalign  Align
	xspan   int
	yspan   int

	x int
	y int

	finalx      int
	finaly      int
	finalwidth  int
	finalheight int
	prefwidth   int
	prefheight  int
}

// NewGrid creates a new Grid with no Controls.
func NewGrid() Grid {
	return &grid{
		indexof: map[Control]int{},
	}
}

// ensures that all (x, y) pairs are 0-based
// also computes g.xmax/g.ymax
func (g *grid) reorigin() {
	xmin := 0
	ymin := 0
	for i := range g.controls {
		if g.controls[i].x < xmin {
			xmin = g.controls[i].x
		}
		if g.controls[i].y < ymin {
			ymin = g.controls[i].y
		}
	}
	xmin = -xmin
	ymin = -ymin
	g.xmax = 0
	g.ymax = 0
	for i := range g.controls {
		g.controls[i].x += xmin
		g.controls[i].y += ymin
		if g.xmax < g.controls[i].x+g.controls[i].xspan {
			g.xmax = g.controls[i].x + g.controls[i].xspan
		}
		if g.ymax < g.controls[i].y+g.controls[i].yspan {
			g.ymax = g.controls[i].y + g.controls[i].yspan
		}
	}
}

func (g *grid) Add(control Control, nextTo Control, side Side, xexpand bool, xalign Align, yexpand bool, yalign Align, xspan int, yspan int) {
	if xspan <= 0 || yspan <= 0 {
		panic(fmt.Errorf("invalid span %dx%d given to Grid.Add()", xspan, yspan))
	}
	cell := gridCell{
		control: control,
		xexpand: xexpand,
		xalign:  xalign,
		yexpand: yexpand,
		yalign:  yalign,
		xspan:   xspan,
		yspan:   yspan,
	}
	if g.parent != nil {
		control.setParent(g.parent)
	}
	// if this is the first control, just add it in directly
	if len(g.controls) != 0 {
		next := g.prev
		if nextTo != nil {
			next = g.indexof[nextTo]
		}
		switch side {
		case West:
			cell.x = g.controls[next].x - cell.xspan
			cell.y = g.controls[next].y
		case North:
			cell.x = g.controls[next].x
			cell.y = g.controls[next].y - cell.yspan
		case East:
			cell.x = g.controls[next].x + g.controls[next].xspan
			cell.y = g.controls[next].y
		case South:
			cell.x = g.controls[next].x
			cell.y = g.controls[next].y + g.controls[next].yspan
		default:
			panic(fmt.Errorf("invalid side %d in Grid.Add()", side))
		}
	}
	g.controls = append(g.controls, cell)
	g.prev = len(g.controls) - 1
	g.indexof[control] = g.prev
	g.reorigin()
}

func (g *grid) Padded() bool {
	return g.padded
}

func (g *grid) SetPadded(padded bool) {
	g.padded = padded
}

func (g *grid) setParent(p *controlParent) {
	g.parent = p
	for _, c := range g.controls {
		c.control.setParent(g.parent)
	}
}

func (g *grid) containerShow() {
	for _, c := range g.controls {
		c.control.containerShow()
	}
}

func (g *grid) containerHide() {
	for _, c := range g.controls {
		c.control.containerHide()
	}
}

// builds the topological cell grid; also makes colwidths and rowheights
func (g *grid) mkgrid() (gg [][]int, colwidths []int, rowheights []int) {
	gg = make([][]int, g.ymax)
	for y := 0; y < g.ymax; y++ {
		gg[y] = make([]int, g.xmax)
		for x := 0; x < g.xmax; x++ {
			gg[y][x] = -1
		}
	}
	for i := range g.controls {
		for y := g.controls[i].y; y < g.controls[i].y+g.controls[i].yspan; y++ {
			for x := g.controls[i].x; x < g.controls[i].x+g.controls[i].xspan; x++ {
				gg[y][x] = i
			}
		}
	}
	return gg, make([]int, g.xmax), make([]int, g.ymax)
}

func (g *grid) resize(x int, y int, width int, height int, d *sizing) {
	if len(g.controls) == 0 {
		// nothing to do
		return
	}

	// -2) get this Grid's padding
	xpadding := d.xpadding
	ypadding := d.ypadding
	if !g.padded {
		xpadding = 0
		ypadding = 0
	}

	// -1) discount padding from width/height
	width -= (g.xmax - 1) * xpadding
	height -= (g.ymax - 1) * ypadding

	// 0) build necessary data structures
	gg, colwidths, rowheights := g.mkgrid()
	xexpand := make([]bool, g.xmax)
	yexpand := make([]bool, g.ymax)

	// 1) compute colwidths and rowheights before handling expansion
	// we only count non-spanning controls to avoid weirdness
	for y := 0; y < len(gg); y++ {
		for x := 0; x < len(gg[y]); x++ {
			i := gg[y][x]
			if i == -1 {
				continue
			}
			w, h := g.controls[i].control.preferredSize(d)
			if g.controls[i].xspan == 1 {
				if colwidths[x] < w {
					colwidths[x] = w
				}
			}
			if g.controls[i].yspan == 1 {
				if rowheights[y] < h {
					rowheights[y] = h
				}
			}
			// save these for step 6
			g.controls[i].prefwidth = w
			g.controls[i].prefheight = h
		}
	}

	// 2) figure out which rows/columns expand but not span
	// we need to know which expanding rows/columns don't span before we can handle the ones that do
	for i := range g.controls {
		if g.controls[i].xexpand && g.controls[i].xspan == 1 {
			xexpand[g.controls[i].x] = true
		}
		if g.controls[i].yexpand && g.controls[i].yspan == 1 {
			yexpand[g.controls[i].y] = true
		}
	}

	// 2) figure out which rows/columns expand that do span
	// the way we handle this is simple: if none of the spanned rows/columns expand, make all rows/columns expand
	for i := range g.controls {
		if g.controls[i].xexpand && g.controls[i].xspan != 1 {
			do := true
			for x := g.controls[i].x; x < g.controls[i].x+g.controls[i].xspan; x++ {
				if xexpand[x] {
					do = false
					break
				}
			}
			if do {
				for x := g.controls[i].x; x < g.controls[i].x+g.controls[i].xspan; x++ {
					xexpand[x] = true
				}
			}
		}
		if g.controls[i].yexpand && g.controls[i].yspan != 1 {
			do := true
			for y := g.controls[i].y; y < g.controls[i].y+g.controls[i].yspan; y++ {
				if yexpand[y] {
					do = false
					break
				}
			}
			if do {
				for y := g.controls[i].y; y < g.controls[i].y+g.controls[i].yspan; y++ {
					yexpand[y] = true
				}
			}
		}
	}

	// 4) compute and assign expanded widths/heights
	nxexpand := 0
	nyexpand := 0
	for x, expand := range xexpand {
		if expand {
			nxexpand++
		} else {
			width -= colwidths[x]
		}
	}
	for y, expand := range yexpand {
		if expand {
			nyexpand++
		} else {
			height -= rowheights[y]
		}
	}
	for x, expand := range xexpand {
		if expand {
			colwidths[x] = width / nxexpand
		}
	}
	for y, expand := range yexpand {
		if expand {
			rowheights[y] = height / nyexpand
		}
	}

	// 5) reset the final coordinates for the next step
	for i := range g.controls {
		g.controls[i].finalx = 0
		g.controls[i].finaly = 0
		g.controls[i].finalwidth = 0
		g.controls[i].finalheight = 0
	}

	// 6) compute cell positions and sizes
	for y := 0; y < g.ymax; y++ {
		curx := 0
		prev := -1
		for x := 0; x < g.xmax; x++ {
			i := gg[y][x]
			if i != -1 && y == g.controls[i].y { // don't repeat this step if the control spans vertically
				if i != prev {
					g.controls[i].finalx = curx
				} else {
					g.controls[i].finalwidth += xpadding
				}
				g.controls[i].finalwidth += colwidths[x]
			}
			curx += colwidths[x] + xpadding
			prev = i
		}
	}
	for x := 0; x < g.xmax; x++ {
		cury := 0
		prev := -1
		for y := 0; y < g.ymax; y++ {
			i := gg[y][x]
			if i != -1 && x == g.controls[i].x { // don't repeat this step if the control spans horizontally
				if i != prev {
					g.controls[i].finaly = cury
				} else {
					g.controls[i].finalheight += ypadding
				}
				g.controls[i].finalheight += rowheights[y]
			}
			cury += rowheights[y] + ypadding
			prev = i
		}
	}

	// 7) everything as it stands now is set for xalign == Fill yalign == Fill; set the correct alignments
	// this is why we saved prefwidth/prefheight above
	for i := range g.controls {
		if g.controls[i].xalign != Fill {
			switch g.controls[i].xalign {
			case RightBottom:
				g.controls[i].finalx += g.controls[i].finalwidth - g.controls[i].prefwidth
			case Center:
				g.controls[i].finalx += (g.controls[i].finalwidth - g.controls[i].prefwidth) / 2
			}
			g.controls[i].finalwidth = g.controls[i].prefwidth // for all three
		}
		if g.controls[i].yalign != Fill {
			switch g.controls[i].yalign {
			case RightBottom:
				g.controls[i].finaly += g.controls[i].finalheight - g.controls[i].prefheight
			case Center:
				g.controls[i].finaly += (g.controls[i].finalheight - g.controls[i].prefheight) / 2
			}
			g.controls[i].finalheight = g.controls[i].prefheight // for all three
		}
	}

	// 8) and FINALLY we draw
	for _, ycol := range gg {
		for _, i := range ycol {
			if i != -1 { // treat empty cells like spaces
				g.controls[i].control.resize(
					g.controls[i].finalx+x, g.controls[i].finaly+y,
					g.controls[i].finalwidth, g.controls[i].finalheight, d)
			}
		}
	}

	return
}

func (g *grid) preferredSize(d *sizing) (width, height int) {
	if len(g.controls) == 0 {
		// nothing to do
		return 0, 0
	}

	// -1) get this Grid's padding
	xpadding := d.xpadding
	ypadding := d.ypadding
	if !g.padded {
		xpadding = 0
		ypadding = 0
	}

	// 0) build necessary data structures
	gg, colwidths, rowheights := g.mkgrid()

	// 1) compute colwidths and rowheights before handling expansion
	// TODO put this in its own function (but careful about the spanning calculation in allocate())
	for y := 0; y < len(gg); y++ {
		for x := 0; x < len(gg[y]); x++ {
			i := gg[y][x]
			if i == -1 {
				continue
			}
			w, h := g.controls[i].control.preferredSize(d)
			// allot equal space in the presence of spanning to keep things sane
			if colwidths[x] < w/g.controls[i].xspan {
				colwidths[x] = w / g.controls[i].xspan
			}
			if rowheights[y] < h/g.controls[i].yspan {
				rowheights[y] = h / g.controls[i].yspan
			}
			// save these for step 6
			g.controls[i].prefwidth = w
			g.controls[i].prefheight = h
		}
	}

	// 2) compute total column width/row height
	colwidth := 0
	rowheight := 0
	for _, w := range colwidths {
		colwidth += w
	}
	for _, h := range rowheights {
		rowheight += h
	}

	// and that's it; just account for padding
	return colwidth + (g.xmax-1) * xpadding,
		rowheight + (g.ymax-1) * ypadding
}

func (g *grid) nTabStops() int {
	n := 0
	for _, c := range g.controls {
		n += c.control.nTabStops()
	}
	return n
}
