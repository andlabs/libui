// 16 december 2016

/*
An attribute list is a doubly linked list of attributes.
The list is kept sorted in increasing order by start position. Whether or not the sort is stable is undefined, so no temporal information should be expected to stay.
Overlapping attributes are not allowed; if an attribute is added that conflicts with an existing one, the existing one is removed.
In addition, the linked list tries to reduce fragmentation: if an attribute is added that just expands another, then there will only be one entry in alist, not two. (TODO does it really?)
The linked list is not a ring; alist->fist->prev == NULL and alist->last->next == NULL.
*/

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
static void attrInsertBefore(struct attrlist *alist, struct attr *a, struct attr *before)
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

// returns the old a->next, for forward iteration
static struct attr *attrUnlink(struct attrlist *alist, struct attr *a)
{
	struct attr *p, *n;

	p = a->prev;
	n = a->next;
	a->prev = NULL;
	a->next = NULL;

	// only item in list?
	if (p == NULL && n == NULL) {
		alist->first = NULL;
		alist->last = NULL;
		return NULL;
	}
	// start of list?
	if (p == NULL) {
		n->prev = NULL;
		alist->first = n;
		return n;
	}
	// end of list?
	if (n == NULL) {
		p->next = NULL;
		alist->last = p;
		return NULL;
	}
	// middle of list
	p->next = n;
	n->prev = p;
	return n;
}

// returns the old a->next, for forward iteration
static struct attr *attrDelete(struct attrlist *alist, struct attr *a)
{
	struct attr *next;

	next = attrUnlink(alist, a);
	uiFree(a);
	return next;
}

// attrDropRange() removes attributes without deleting characters.
// 
// If the attribute needs no change, then nothing is done.
// 
// If the attribute needs to be deleted, it is deleted.
// 
// If the attribute only needs to be resized at the end, it is adjusted.
// 
// If the attribute only needs to be resized at the start, it is adjusted, unlinked, and returned in tail.
// 
// Otherwise, the attribute needs to be split. The existing attribute is adjusted to make the left half and a new attribute with the right half. This attribute is kept unlinked and returned in tail.
// 
// In all cases, the return value is the next attribute to look at in a forward sequential loop.
static struct attr *attrDropRange(struct attrlist *alist, struct attr *a, size_t start, size_t end, struct attr **tail)
{
	struct attr *b;

	// always pre-initialize tail to NULL
	*tail = NULL;

	if (!attrRangeIntersect(a, &start, &end))
		// out of range; nothing to do
		return a->next;

	// just outright delete the attribute?
	if (a->start == start && a->end == end)
		return attrDelete(alist, a);

	// only chop off the start or end?
	if (a->start == start) {		// chop off the end
		a->end = end;
		return a->next;
	}
	if (a->end == end) {			// chop off the start
		a->start = start;
		*tail = a;
		return attrUnlink(attr, a);
	}

	// we'll need to split the attribute into two
	b = uiNew(struct attr);
	b->type = a->type;
	b->val = a->val;
	b->start = end;
	b->end = a->end;
	*tail = b;

	a->end = start;
	return a->next;
}

static void attrGrow(struct attrlist *alist, struct attr *a, size_t start, size_t end)
{
	struct attr *before;

	// adjusting the end is simple: if it ends before our new end, just set the new end
	if (a->end < end)
		a->end = end;

	// adjusting the start is harder
	// if the start is before our new start, we are done
	// otherwise, we have to move the start back AND reposition the attribute to keep the sorted order
	if (a->start <= start)
		return;
	a->start = start;
	attrUnlink(alist, a);
	for (before = alist->first; before != NULL; before = before->next)
		if (before->start > a->start)
			break;
	attrInsertBefore(alist, a, before);
}

void attrlistInsertAt(struct attrlist *alist, uiAttribute type, uintptr_t val, size_t start, size_t end)
{
	struct attr *a;
	struct attr *before;
	struct attr *tail = NULL;
	int split = 0;

	// first, figure out where in the list this should go
	// in addition, if this attribute overrides one that already exists, split that one apart so this one can take over
	before = alist->first;
	while (before != NULL) {
		size_t lstart, lend;

		// once we get to the first point after start, we know where to insert
		if (before->start > start)
			break;

		// if we have already split a prior instance of this attribute, don't bother doing it again
		if (split)
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
		// TODO will this cause problems with fonts?
		// TODO will this reduce fragmentation if we first add from 0 to 2 and then from 2 to 4? or do we have to do that separately?
		if (before->val == val) {
			attrGrow(alist, a, start, end);
			return;
		}
		// okay the values are different; we need to split apart
		before = attrDropRange(alist, a, start, end, &tail);
		split = 1;
		continue;

	next:
		before = before->next;
	}

	// if we got here, we know we have to add the attribute before before
	a = uiNew(struct attr);
	a->type = type;
	a->val = val;
	a->start = start;
	a->end = end;
	attrInsertBefore(alist, a, before);

	// and finally, if we split, insert the remainder
	if (tail == NULL)
		return;
	// note we start at before; it won't be inserted before that by the sheer nature of how the code above works
	for (; before != NULL; before = before->next)
		if (before->start > tail->start)
			break;
	attrInsertBefore(alist, tail, before);
}

void attrlistInsertCharacters(struct attrlist *alist, size_t start, size_t end)
{
}

// The attributes are those of character start - 1.
// If start == 0, the attributes are those of character 0.
void attrlistInsertCharactersExtendingAttributes(struct attrlist *alist, size_t start, size_t end)
{
}

void attrlistRemoveAttribute(struct attrlist *alist, uiAttribute type, size_t start, size_t end)
{
}

void attrlistRemoveAttributes(struct attrlist *alist, size_t start, size_t end)
{
}

void attrlistRemoveCharacters(struct attrlist *alist, size_t start, size_t end)
{
}
