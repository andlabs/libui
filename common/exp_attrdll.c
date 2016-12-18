// 16 december 2016

struct attr {
	uiAttribute type;
	uintptr_t val;
	size_t start;
	size_t end;
	struct attr *prev;
	struct attr *next;
};

struct attrlist {
	struct attr *first;
	struct attr *last;
};

// if before is NULL, add to the end of the list
static void linkInsertBefore(struct attrlist *alist, struct attr *what, struct attr *before)
{
	// if the list is empty, this is the first item
	if (alist->first == NULL) {
		alist->first = a;
		alist->last = a;
		return;
	}

	// add to the end
	if (before == NULL) {
		struct attr *oldlast;

		oldlast = alist->last;
		alist->last = a;
		a->prev = oldlast;
		oldlast->next = a;
		return;
	}

	// add to the beginning
	if (before == alist->first) {
		struct attr *oldfirst;

		oldfirst = alist->first;
		alist->first = a;
		oldfirst->prev = a;
		a->next = oldfirst;
		return;
	}

	// add to the middle
	a->prev = before->prev;
	a->next = before;
	before->prev = a;
	a->prev->next = a;
}

static int attrHasPos(struct attr *a, size_t pos)
{
	if (pos < a->start)
		return 0;
	return pos < a->end;
}

// returns 1 if there was an intersection and 0 otherwise
static int attrRangeIntersect(struct attr *a, size_t *start, size_t *end)
{
	// is the range outside a entirely?
	if (*start >= a->end)
		return 0;
	if (*end < a->start)
		return 0;

	// okay, so there is an overlap
	// compute the intersection
	if (*start < a->start)
		*start = a->start;
	if (*end > a->end)
		*end = a->end;
	return 1;
}

#if 0
void attrlistInsertAt(struct attrlist *alist, uiAttribute type, uintptr_t val, size_t start, size_t end)
{
	struct attr *a;
	struct attr *before;

	a = uiNew(struct attr);
	a->type = type;
	a->val = val;
	a->start = start;
	a->end = end;

	// place a before the first element that starts after a does
	for (before = alist->first; before != NULL; before = before->next) {
		size_t lstart, lend;

		if (before->start > a->start)
			break;
		// if this attribute overrides another, we have to split
		lstart = start;
		lend = end;
		if (!attrRangeIntersect(before, &lstart, &lend))
			continue;
		if (before->type != type)
			continue;
		if (before->val == val)
			continue;
		// TODO now split around start/end and drop the overlap
	}
	linkInsertBefore(alist, a, before);
	// TODO see if adding this attribute leads to a fragmented contiguous run
}
#endif

void attrlistInsertAt(struct attrlist *alist, uiAttribute type, uintptr_t val, size_t start, size_t end)
{
	struct attr *a;
	struct attr *before;
	struct attr *tail = NULL;

	// first, figure out where in the list this should go
	// in addition, if this attribute overrides one that already exists, split that one apart so this one can take over
	before = alist->first;
	while (before != NULL) {
		size_t lstart, lend;

		// once we get to the first point after start, we know where to insert
		if (before->start > start)
			break;

		// if we have already split a prior instance of this attribute, don't bother doing it again
		if (tail != NULL)
			goto next;

		// should we split this?
		if (before->type != type)
			goto next;
		lstart = start;
		lend = end;
		if (!attrRangeIntersects(before, &lstart, &lend))
			goto next;

		// okay so this might conflict; if the val is the same as the one we want, we need to expand the existing attribute, not fragment anything
		// TODO this might cause problems with system specific attributes, if we support those; maybe also user-specific?
		if (before->val == val) {
			// TODO
		}
		// okay the values are different; we need to split apart
		// TODO

	next:
		before = before->next;
	}

	// if we got here, we know we have to add the attribute before before
	a = uiNew(struct attr);
	a->type = type;
	a->val = val;
	a->start = start;
	a->end = end;
	linkInsertBefore(alist, a, before);

	// and finally, if we split, insert the remainder
	if (tail == NULL)
		return;
	// note we start at before; it won't be inserted before that by the sheer nature of how the code above works
	for (; before != NULL; before = before->next)
		if (before->start > tail->start)
			break;
	linkInsertBefore(alist, tail, before);
}
