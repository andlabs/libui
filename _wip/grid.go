func (g *grid) preferredSize(d *sizing) (width, height int) {}

func (g *grid) nTabStops() int {
	n := 0
	for _, c := range g.controls {
		n += c.control.nTabStops()
	}
	return n
}
