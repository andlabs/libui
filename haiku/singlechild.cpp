// 19 november 2015
#include "uipriv_haiku.hpp"

// singlechild.cpp is like child.c/child.m in the other ports, except it only handles single children with an optional margin.

struct singleChild {
	uiControl *c;
	BView *view;
	BGroupLayout *box;
	BLayoutItem *item;
	BAlignment oldalign;
};

struct singleChild *newSingleChild(uiControl *c, uiControl *parent, void (*attach)(void *, BLayoutItem *), void *attachTo)
{
	struct singleChild *s;

	if (c == NULL)
		return NULL;

	s = uiNew(struct singleChild);
	s->c = c;
	s->view = (BView *) uiControlHandle(s->c);
	s->oldalign = s->view->ExplicitAlignment();

	uiControlSetParent(s->c, parent);

	s->box = new BGroupLayout(B_HORIZONTAL, 0);
	// TODO TODO TODO TODO
	// currently BLayout won't let you add a view to a layout that isn't attached to a view
	// that is, if you want to add a view to a layout, there must already be a parent view
	// request this behavior to be changed
	(*attach)(attachTo, s->box);

	s->view->SetExplicitAlignment(BAlignment(B_ALIGN_USE_FULL_WIDTH, B_ALIGN_USE_FULL_HEIGHT));
	s->item = s->box->AddView(s->view, 1.0);

	// and set it on the box as well
	// this way it fills the entire space
	s->box->SetExplicitAlignment(BAlignment(B_ALIGN_USE_FULL_WIDTH, B_ALIGN_USE_FULL_HEIGHT));

	return s;
}

void singleChildRemove(struct singleChild *s)
{
	s->box->RemoveItem(s->item);
	delete s->item;
	delete s->box;
	uiControlSetParent(s->c, NULL);
	s->view->SetExplicitAlignment(s->oldalign);
	uiFree(s);
}

void singleChildDestroy(struct singleChild *s)
{
	uiControl *child;

	child = s->c;
	singleChildRemove(s);
	uiControlDestroy(child);
}

// this is of the box itself, not of the child
// it is used to add the child to the parent layout
BLayoutItem *singleChildLayoutItem(struct singleChild *s)
{
	return s->box;
}

void singleChildUpdateState(struct singleChild *s)
{
	controlUpdateState(s->c);
}

void singleChildSetMargined(struct singleChild *s, float inset)
{
	s->box->SetInsets(inset, inset, inset, inset);
}
