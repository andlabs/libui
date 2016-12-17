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

void attrlistInsertAt(struct attrlist *alist, uiAttribute type, uintptr_t val, size_t start, size_t end)
{
	struct attr *a;
	struct attr *after;

	a = uiNew(struct attr);
	a->type = type;
	a->val = val;
	a->start = start;
	a->end = end;

	// place a before the first element that starts after a does
	for (before = alist->first; before != NULL; before = before->next)
		if (before->start > a->start)
			break;
	// TODO this does not handle cases where the attribute overwrites an existing attribute
	linkInsertBefore(alist, a, before);
}
