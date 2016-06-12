	// if a row or column only contains emptys and spanning cells of a opposite-direction spannings, remove it by duplicating the previous row or column
	BOOL onlyEmptyAndSpanning;
	for (y = 0; y < ycount; y++) {
		onlyEmptyAndSpanning = YES;
		for (x = 0; x < xcount; x++)
			if (gg[y][x] != -1) {
				gc = (gridChild *) [self->children objectAtIndex:gg[y][x]];
				if (gc.yspan == 1 || gc.top - ymin == y) {
					onlyEmptyAndSpanning = NO;
					break;
				}
			}
		if (onlyEmptyAndSpanning)
			for (x = 0; x < xcount; x++)
				gg[y][x] = gg[y - 1][x];
	}
	for (x = 0; x < xcount; x++) {
		onlyEmptyAndSpanning = YES;
		for (y = 0; y < ycount; y++)
			if (gg[y][x] != -1) {
				gc = (gridChild *) [self->children objectAtIndex:gg[y][x]];
				if (gc.xspan == 1 || gc.left - xmin == x) {
					onlyEmptyAndSpanning = NO;
					break;
				}
			}
		if (onlyEmptyAndSpanning)
			for (y = 0; y < ycount; y++)
				gg[y][x] = gg[y][x - 1];
	}
