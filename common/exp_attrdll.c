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

void attrlistInsertAt(struct attrlist *alist, uiAttribute type, uintptr_t val, size_t start, size_t end)
{
	struct attr *a;
	struct attr *after;

	a = uiNew(struct attr);
	a->type = type;
	a->val = val;
	a->start = start;
	a->end = end;

	if (alist->first == NULL) {		// empty list
		alist->first = a;
		alist->last = a;
		return;
	}

	// place a before the first element that starts after a does
	for (before = alist->first; before != NULL; before = before->next)
		if (before->start > a->start)
			break;
	if (before == NULL) {		// if there is none, add a to the end
		alist->last->next = a;
		a->prev = alist->last;
		alist->last = a;
		return;
	}

	if (before == alist->first)
		alist->first = a;
	else		// not the first; hook up our new prev
		before->prev->next = a;
	a->next = before;
	a->prev = before->prev;
	before->prev = a;
}
