	NSView **colviews, **rowviews;
	// now go through every row and column and extract SOME view from that row and column for the inner constraints
	// if it turns out that a row or column is totally empty, duplicate the one to the left (this has the effect of collapsing empty rows)
	// note that the edges cannot be empty because we built a smallest fitting rectangle way back in step 1
	colviews = (NSView **) uiAlloc(xcount * sizeof (NSView *), "NSView *[]");
	for (x = 0; x < xcount; x++) {
		for (y = 0; y < ycount; y++)
			if (gg[y][x] != -1) {
				gc = (gridChild *) [self->children objectAtIndex:gg[y][x]];
				colviews[x] = [gc view];
				break;
			}
		if (colviews[x] == nil)
			colviews[x] = colviews[x - 1];
	}
	rowviews = (NSView **) uiAlloc(ycount * sizeof (NSView *), "NSView *[]");
	for (y = 0; y < ycount; y++) {
		for (x = 0; x < xcount; x++)
			if (gg[y][x] != -1) {
				gc = (gridChild *) [self->children objectAtIndex:gg[y][x]];
				rowviews[y] = [gc view];
				break;
			}
		if (rowviews[y] == nil)
			rowviews[y] = rowviews[y - 1];
	}

	// now string all the views together
	for (gc in self->children) {
		if (gc.left != xmin) {
			c = mkConstraint([gc view], NSLayoutAttributeLeading,
				NSLayoutRelationEqual,
				colviews[(gc.left - 1) - xmin], NSLayoutAttributeTrailing,
				1, padding,
				@"uiGrid leading constraint");
			[self addConstraint:c];
			[self->inBetweens addObject:c];
		}
		if (gc.top != ymin) {
			c = mkConstraint([gc view], NSLayoutAttributeTop,
				NSLayoutRelationEqual,
				rowviews[(gc.top - 1) - ymin], NSLayoutAttributeBottom,
				1, padding,
				@"uiGrid top constraint");
			[self addConstraint:c];
			[self->inBetweens addObject:c];
		}
		if ((gc.left + gc.xspan) != xmax) {
			c = mkConstraint([gc view], NSLayoutAttributeTrailing,
				NSLayoutRelationEqual,
				colviews[(gc.left + gc.xspan) - xmin], NSLayoutAttributeLeading,
				1, -padding,
				@"uiGrid trailing constraint");
			[self addConstraint:c];
			[self->inBetweens addObject:c];
		}
		if ((gc.top + gc.yspan) != ymax) {
			c = mkConstraint([gc view], NSLayoutAttributeBottom,
				NSLayoutRelationEqual,
				rowviews[(gc.top + gc.yspan) - ymin], NSLayoutAttributeTop,
				1, -padding,
				@"uiGrid bottom constraint");
			[self addConstraint:c];
			[self->inBetweens addObject:c];
		}
	}
