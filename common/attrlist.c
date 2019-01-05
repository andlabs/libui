// 16 december 2016
#include "../ui.h"
#include "uipriv.h"
#include "attrstr.h"

/*
An attribute list is a doubly linked list of attributes.
Attribute start positions are inclusive and attribute end positions are exclusive (or in other words, [start, end)).
The list is kept sorted in increasing order by start position. Whether or not the sort is stable is undefined, so no temporal information should be expected to stay.
Overlapping attributes are not allowed; if an attribute is added that conflicts with an existing one, the existing one is removed.
In addition, the linked list tries to reduce fragmentation: if an attribute is added that just expands another, then there will only be one entry in alist, not two. (TODO does it really?)
The linked list is not a ring; alist->fist->prev == NULL and alist->last->next == NULL.
TODO verify that this disallows attributes of length zero
*/

struct attr {
	uiAttribute *val;
	size_t start;
	size_t end;
	struct attr *prev;
	struct attr *next;
};

struct uiprivAttrList {
	struct attr *first;
	struct attr *last;
};

// if before is NULL, add to the end of the list
static void attrInsertBefore(uiprivAttrList *alist, struct attr *a, struct attr *before)
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
static struct attr *attrUnlink(uiprivAttrList *alist, struct attr *a)
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
static struct attr *attrDelete(uiprivAttrList *alist, struct attr *a)
{
	struct attr *next;

	next = attrUnlink(alist, a);
	uiprivAttributeRelease(a->val);
	uiprivFree(a);
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
static struct attr *attrDropRange(uiprivAttrList *alist, struct attr *a, size_t start, size_t end, struct attr **tail)
{
	struct attr *b;

	// always pre-initialize tail to NULL
	*tail = NULL;

	if (!attrRangeIntersect(a, &start, &end))
		// out of range; nothing to do
		return a->next;

	// just outright delete the attribute?
	// the inequalities handle attributes entirely inside the range
	// if both are equal, the attribute's range is equal to the range
	if (a->start >= start && a->end <= end)
		return attrDelete(alist, a);

	// only chop off the start or end?
	if (a->start == start) {			// chop off the start
		// we are dropping the left half, so set a->start and unlink
		a->start = end;
		*tail = a;
		return attrUnlink(alist, a);
	}
	if (a->end == end) {				// chop off the end
		// we are dropping the right half, so just set a->end
		a->end = start;
		return a->next;
	}

	// we'll need to split the attribute into two
	b = uiprivNew(struct attr);
	b->val = uiprivAttributeRetain(a->val);
	b->start = end;
	b->end = a->end;
	*tail = b;

	a->end = start;
	return a->next;
}

static void attrGrow(uiprivAttrList *alist, struct attr *a, size_t start, size_t end)
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

// returns the right side of the split, which is unlinked, or NULL if no split was done
static struct attr *attrSplitAt(uiprivAttrList *alist, struct attr *a, size_t at)
{
	struct attr *b;

	// no splittng needed?
	// note the equality: we don't need to split at start or end
	// in the end case, the last split point is at - 1; at itself is outside the range, and at - 1 results in the right hand side having length 1
	if (at <= a->start)
		return NULL;
	if (at >= a->end)
		return NULL;

	b = uiprivNew(struct attr);
	b->val = uiprivAttributeRetain(a->val);
	b->start = at;
	b->end = a->end;

	a->end = at;
	return b;
}

// attrDeleteRange() removes attributes while deleting characters.
// 
// If the attribute does not include the deleted range, then nothing is done (though the start and end are adjusted as necessary).
// 
// If the attribute needs to be deleted, it is deleted.
// 
// Otherwise, the attribute only needs the start or end deleted, and it is adjusted.
// 
// In all cases, the return value is the next attribute to look at in a forward sequential loop.
// TODO rewrite this comment
static struct attr *attrDeleteRange(uiprivAttrList *alist, struct attr *a, size_t start, size_t end)
{
	size_t ostart, oend;
	size_t count;

	ostart = start;
	oend = end;
	count = oend - ostart;

	if (!attrRangeIntersect(a, &start, &end)) {
		// out of range
		// adjust if necessary
		if (a->start >= ostart)
			a->start -= count;
		if (a->end >= oend)
			a->end -= count;
		return a->next;
	}

	// just outright delete the attribute?
	// the inequalities handle attributes entirely inside the range
	// if both are equal, the attribute's range is equal to the range
	if (a->start >= start && a->end <= end)
		return attrDelete(alist, a);

	// only chop off the start or end?
	if (a->start == start) {			// chop off the start
		// if we weren't adjusting positions this would just be setting a->start to end
		// but since this is deleting from the start, we need to adjust both by count
		a->start = end - count;
		a->end -= count;
		return a->next;
	}
	if (a->end == end) {				// chop off the end
		// a->start is already good
		a->end = start;
		return a->next;
	}

	// in this case, the deleted range is inside the attribute
	// we can clear it by just removing count from a->end
	a->end -= count;
	return a->next;
}

uiprivAttrList *uiprivNewAttrList(void)
{
	return uiprivNew(uiprivAttrList);
}

void uiprivFreeAttrList(uiprivAttrList *alist)
{
	struct attr *a, *next;

	a = alist->first;
	while (a != NULL) {
		next = a->next;
		uiprivAttributeRelease(a->val);
		uiprivFree(a);
		a = next;
	}
	uiprivFree(alist);
}

void uiprivAttrListInsertAttribute(uiprivAttrList *alist, uiAttribute *val, size_t start, size_t end)
{
	struct attr *a;
	struct attr *before;
	struct attr *tail = NULL;
	int split = 0;
	uiAttributeType valtype;

	// first, figure out where in the list this should go
	// in addition, if this attribute overrides one that already exists, split that one apart so this one can take over
	before = alist->first;
	valtype = uiAttributeGetType(val);
	while (before != NULL) {
		size_t lstart, lend;

		// once we get to the first point after start, we know where to insert
		if (before->start > start)
			break;

		// if we have already split a prior instance of this attribute, don't bother doing it again
		if (split)
			goto next;

		// should we split this attribute?
		if (uiAttributeGetType(before->val) != valtype)
			goto next;
		lstart = start;
		lend = end;
		if (!attrRangeIntersect(before, &lstart, &lend))
			goto next;

		// okay so this might conflict; if the val is the same as the one we want, we need to expand the existing attribute, not fragment anything
		// TODO will this reduce fragmentation if we first add from 0 to 2 and then from 2 to 4? or do we have to do that separately?
		if (uiprivAttributeEqual(before->val, val)) {
			attrGrow(alist, before, start, end);
			return;
		}
		// okay the values are different; we need to split apart
		before = attrDropRange(alist, before, start, end, &tail);
		split = 1;
		continue;

	next:
		before = before->next;
	}

	// if we got here, we know we have to add the attribute before before
	a = uiprivNew(struct attr);
	a->val = uiprivAttributeRetain(val);
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

void uiprivAttrListInsertCharactersUnattributed(uiprivAttrList *alist, size_t start, size_t count)
{
	struct attr *a;
	struct attr *tails = NULL;

	// every attribute before the insertion point can either cross into the insertion point or not
	// if it does, we need to split that attribute apart at the insertion point, keeping only the old attribute in place, adjusting the new tail, and preparing it for being re-added later
	for (a = alist->first; a != NULL; a = a->next) {
		struct attr *tail;

		// stop once we get to the insertion point
		if (a->start >= start)
			break;
		// only do something if overlapping
		if (!attrHasPos(a, start))
			continue;

		tail = attrSplitAt(alist, a, start);
		// adjust the new tail for the insertion
		tail->start += count;
		tail->end += count;
		// and queue it for re-adding later
		// we can safely use tails as if it was singly-linked since it's just a temporary list; we properly merge them back in below and they'll be doubly-linked again then
		// TODO actually we could probably save some time by making then doubly-linked now and adding them in one fell swoop, but that would make things a bit more complicated...
		tail->next = tails;
		tails = tail;
	}

	// at this point in the attribute list, we are at or after the insertion point
	// all the split-apart attributes will be at the insertion point
	// therefore, we can just add them all back now, and the list will still be sorted correctly
	while (tails != NULL) {
		struct attr *next;

		// make all the links NULL before insertion, just to be safe
		next = tails->next;
		tails->next = NULL;
		attrInsertBefore(alist, tails, a);
		tails = next;
	}

	// every remaining attribute will be either at or after the insertion point
	// we just need to move them ahead
	for (; a != NULL; a = a->next) {
		a->start += count;
		a->end += count;
	}
}

// The attributes are those of character start - 1.
// If start == 0, the attributes are those of character 0.
/*
This is an obtuse function. Here's some diagrams to help.

Given the input string
	abcdefghi (positions: 012345678 9)
and attribute set
	red start 0 end 3
	bold start 2 end 6
	underline start 5 end 8
or visually:
	012345678 9
	rrr------
	--bbbb---
	-----uuu-
If we insert qwe to result in positions 0123456789AB C:

before 0, 1, 2 (grow the red part, move everything else down)
	red -> start 0 (no change) end 3+3=6
	bold -> start 2+3=5 end 6+3=9
	underline -> start 5+3=8 end 8+3=B
before 3 (grow red and bold, move underline down)
	red -> start 0 (no change) end 3+3=6
	bold -> start 2 (no change) end 6+3=9
	underline -> start 5+3=8 end 8+3=B
before 4, 5 (keep red, grow bold, move underline down)
	red -> start 0 (no change) end 3 (no change)
	bold -> start 2 (no change) end 6+3=9
	underline -> start 5+3=8 end 8+3=B
before 6 (keep red, grow bold and underline)
	red -> start 0 (no change) end 3 (no change)
	bold -> start 2 (no change) end 6+3=9
	underline -> start 5 (no change) end 8+3=B
before 7, 8 (keep red and bold, grow underline)
	red -> start 0 (no change) end 3 (no change)
	bold -> start 2 (no change) end 6 (no change)
	underline -> start 5 (no change) end 8+3=B
before 9 (keep all three)
	red -> start 0 (no change) end 3 (no change)
	bold -> start 2 (no change) end 6 (no change)
	underline -> start 5 (no change) end 8 (no change)

result:
          0 1 2 3 4 5 6 7 8 9
      red E E E e n n n n n n
     bold s s S E E E e n n n
underline s s s s s S E E e n
N = none
E = end only
S = start and end
uppercase = in original range, lowercase = not

which results in our algorithm:
	for each attribute
		if start < insertion point
			move start up
		else if start == insertion point
			if start != 0
				move start up
		if end <= insertion point
			move end up
*/
// TODO does this ensure the list remains sorted?
void uiprivAttrListInsertCharactersExtendingAttributes(uiprivAttrList *alist, size_t start, size_t count)
{
	struct attr *a;

	for (a = alist->first; a != NULL; a = a->next) {
		if (a->start < start)
			a->start += count;
		else if (a->start == start && start != 0)
			a->start += count;
		if (a->end <= start)
			a->end += count;
	}
}

// TODO replace at point with — replaces with first character's attributes

void uiprivAttrListRemoveAttribute(uiprivAttrList *alist, uiAttributeType type, size_t start, size_t end)
{
	struct attr *a;
	struct attr *tails = NULL;		// see uiprivAttrListInsertCharactersUnattributed() above
	struct attr *tailsAt = NULL;

	a = alist->first;
	while (a != NULL) {
		size_t lstart, lend;
		struct attr *tail;

		// this defines where to re-attach the tails
		// (all the tails will have their start at end, so we can just insert them all before tailsAt)
		if (a->start >= end) {
			tailsAt = a;
			// and at this point we're done, so
			break;
		}
		if (uiAttributeGetType(a->val) != type)
			goto next;
		lstart = start;
		lend = end;
		if (!attrRangeIntersect(a, &lstart, &lend))
			goto next;
		a = attrDropRange(alist, a, start, end, &tail);
		if (tail != NULL) {
			tail->next = tails;
			tails = tail;
		}
		continue;

	next:
		a = a->next;
	}

	while (tails != NULL) {
		struct attr *next;

		// make all the links NULL before insertion, just to be safe
		next = tails->next;
		tails->next = NULL;
		attrInsertBefore(alist, tails, a);
		tails = next;
	}
}

// TODO merge this with the above
void uiprivAttrListRemoveAttributes(uiprivAttrList *alist, size_t start, size_t end)
{
	struct attr *a;
	struct attr *tails = NULL;		// see uiprivAttrListInsertCharactersUnattributed() above
	struct attr *tailsAt = NULL;

	a = alist->first;
	while (a != NULL) {
		size_t lstart, lend;
		struct attr *tail;

		// this defines where to re-attach the tails
		// (all the tails will have their start at end, so we can just insert them all before tailsAt)
		if (a->start >= end) {
			tailsAt = a;
			// and at this point we're done, so
			break;
		}
		lstart = start;
		lend = end;
		if (!attrRangeIntersect(a, &lstart, &lend))
			goto next;
		a = attrDropRange(alist, a, start, end, &tail);
		if (tail != NULL) {
			tail->next = tails;
			tails = tail;
		}
		continue;

	next:
		a = a->next;
	}

	while (tails != NULL) {
		struct attr *next;

		// make all the links NULL before insertion, just to be safe
		next = tails->next;
		tails->next = NULL;
		attrInsertBefore(alist, tails, a);
		tails = next;
	}
}

void uiprivAttrListRemoveCharacters(uiprivAttrList *alist, size_t start, size_t end)
{
	struct attr *a;

	a = alist->first;
	while (a != NULL)
		a = attrDeleteRange(alist, a, start, end);
}

void uiprivAttrListForEach(const uiprivAttrList *alist, const uiAttributedString *s, uiAttributedStringForEachAttributeFunc f, void *data)
{
	struct attr *a;
	uiForEach ret;

	for (a = alist->first; a != NULL; a = a->next) {
		ret = (*f)(s, a->val, a->start, a->end, data);
		if (ret == uiForEachStop)
			// TODO for all: break or return?
			break;
	}
}
