	// now find all horizontally adjacent views and string them together
	for (y = 0; y < ycount; y++)
		for (x = 0; x < xcount - 1; x++) {
			if (gg[y][x] == -1)
				continue;
			if (gg[y][x + 1] == -1)
				continue;
			if (gg[y][x] == gg[y][x + 1])		// spanning
				continue;
			gc = (gridChild *) [self->children objectAtIndex:gg[y][x]];
			firstView = [gc view];
			gc = (gridChild *) [self->children objectAtIndex:gg[y][x + 1]];
			c = mkConstraint(firstView, NSLayoutAttributeTrailing,
				NSLayoutRelationEqual,
				[gc view], NSLayoutAttributeLeading,
				1, -padding,
				@"uiGrid inside trailing attribute");
			[self addConstraint:c];
			[self->inBetweens addObject:c];
		}
	// and same for vertically adjacent
	for (x = 0; x < xcount; x++)
		for (y = 0; y < ycount - 1; y++) {
			if (gg[y][x] == -1)
				continue;
			if (gg[y + 1][x] == -1)
				continue;
			if (gg[y][x] == gg[y + 1][x])		// spanning
				continue;
			gc = (gridChild *) [self->children objectAtIndex:gg[y][x]];
			firstView = [gc view];
			gc = (gridChild *) [self->children objectAtIndex:gg[y + 1][x]];
			c = mkConstraint(firstView, NSLayoutAttributeBottom,
				NSLayoutRelationEqual,
				[gc view], NSLayoutAttributeTop,
				1, -padding,
				@"uiGrid inside bottom attribute");
			[self addConstraint:c];
			[self->inBetweens addObject:c];
		}
