	// now put all the views in the same row and column together
	for (x = 0; x < xcount; x++) {
		[set removeAllObjects];
		for (y = 0; y < ycount; y++)
			[set addObject:[NSNumber numberWithInt:gg[y][x]]];
		first = YES;
		for (number in set) {
			if ([number intValue] == -1)
				continue;
			gc = (gridChild *) [self->children objectAtIndex:[number intValue]];
			if (first) {
				firstView = [gc view];
				first = NO;
				continue;
			}
			c = mkConstraint([gc view], NSLayoutAttributeLeading,
				NSLayoutRelationEqual,
				firstView, NSLayoutAttributeLeading,
				1, 0,
				@"uiGrid column left edge constraint");
			[self addConstraint:c];
			[self->edges addObject:c];
		}
	}
	for (y = 0; y < ycount; y++) {
		[set removeAllObjects];
		for (x = 0; x < xcount; x++)
			[set addObject:[NSNumber numberWithInt:gg[y][x]]];
		first = YES;
		for (number in set) {
			if ([number intValue] == -1)
				continue;
			gc = (gridChild *) [self->children objectAtIndex:[number intValue]];
			if (first) {
				firstView = [gc view];
				first = NO;
				continue;
			}
			c = mkConstraint([gc view], NSLayoutAttributeTop,
				NSLayoutRelationEqual,
				firstView, NSLayoutAttributeTop,
				1, 0,
				@"uiGrid row top edge constraint");
			[self addConstraint:c];
			[self->edges addObject:c];
		}
	}
