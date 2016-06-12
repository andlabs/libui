	// now figure out which rows and columns really expand
	hexpand = (BOOL *) uiAlloc(xcount * sizeof (BOOL), "BOOL[]");
	vexpand = (BOOL *) uiAlloc(ycount * sizeof (BOOL), "BOOL[]");
	// first, which don't span
	for (gc in self->children) {
		if (gc.hexpand && gc.xspan == 1)
			hexpand[gc.left - xmin] = YES;
		if (gc.vexpand && gc.yspan == 1)
			vexpand[gc.top - ymin] = YES;
	}
	// second, which do span
	// the way we handle this is simple: if none of the spanned rows/columns expand, make all rows/columns expand
	for (gc in self->children) {
		if (gc.hexpand && gc.xspan != 1) {
			doit = YES;
			for (x = gc.left; x < gc.left + gc.xspan; x++)
				if (hexpand[x - xmin]) {
					doit = NO;
					break;
				}
			if (doit)
				for (x = gc.left; x < gc.left + gc.xspan; x++)
					hexpand[x - xmin] = YES;
		}
		if (gc.vexpand && gc.yspan != 1) {
			doit = YES;
			for (y = gc.top; y < gc.top + gc.yspan; y++)
				if (vexpand[y - ymin]) {
					doit = NO;
					break;
				}
			if (doit)
				for (y = gc.top; y < gc.top + gc.yspan; y++)
					vexpand[y - ymin] = YES;
		}
	}
