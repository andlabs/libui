// 18 november 2015
#include <vector>
#include "uipriv_haiku.hpp"
using namespace std;

struct boxchild {
	uiControl *c;
	BAlignment oldalign;
	bool stretchy;
	BLayoutItem *item;
};

struct uiBox {
	uiHaikuControl c;
	BGroupLayout *layout;
	vector<struct boxchild> *controls;
	int vertical;
	int padded;
};

static void onDestroy(uiBox *b);

uiHaikuDefineControlWithOnDestroy(
	uiBox,								// type name
	uiBoxType,							// type function
	layout,								// handle
	onDestroy(hthis);						// on destroy
)

static void onDestroy(uiBox *b)
{
	struct boxchild bc;

	while (b->controls->size() != 0) {
		bc = b->controls->back();
		uiControlSetParent(bc.c, NULL);
		uiControlDestroy(bc.c);
		b->controls->pop_back();
	}
	delete b->controls;
}

static void boxContainerUpdateState(uiControl *c)
{
	uiBox *b = uiBox(c);
	struct boxchild bc;
	uintmax_t i;

	for (i = 0; i < b->controls->size(); i++) {
		bc = b->controls->at(i);
		controlUpdateState(bc.c);
	}
}

#define isStretchy(bc) bc.stretchy

void uiBoxAppend(uiBox *b, uiControl *c, int stretchy)
{
	struct boxchild bc;
	BView *view;
	BAlignment alignment;
	float weight;

	bc.c = c;
	view = (BView *) uiControlHandle(bc.c);
	bc.oldalign = view->ExplicitAlignment();
	bc.stretchy = stretchy != 0;

	alignment.horizontal = B_ALIGN_USE_FULL_WIDTH;
	alignment.vertical = B_ALIGN_USE_FULL_HEIGHT;
	weight = 0.0;
	if (isStretchy(bc))
		weight = 1.0;
	else {
		if (b->vertical)
			alignment.vertical = B_ALIGN_TOP;
		else
			alignment.horizontal = B_ALIGN_LEFT;
	}

	uiControlSetParent(bc.c, uiControl(b));
	view->SetExplicitAlignment(alignment);
	bc.item = b->layout->AddView(view, weight);

	b->controls->push_back(bc);
}

void uiBoxDelete(uiBox *b, uintmax_t index)
{
	struct boxchild bc;
	BView *view;

	bc = b->controls->back();
	b->controls->pop_back();

	b->layout->RemoveItem(bc.item);
	delete bc.item;

	view = (BView *) uiControlHandle(bc.c);
	view->SetExplicitAlignment(bc.oldalign);

	uiControlSetParent(bc.c, NULL);
}

int uiBoxPadded(uiBox *b)
{
	return b->padded;
}

void uiBoxSetPadded(uiBox *b, int padded)
{
	b->padded = padded;
	if (b->padded)
		b->layout->SetSpacing(B_USE_DEFAULT_SPACING);
	else
		b->layout->SetSpacing(0);
}

static uiBox *finishNewBox(orientation o)
{
	uiBox *b;

	b = (uiBox *) uiNewControl(uiBoxType());

	b->layout = new BGroupLayout(o, 0);

	b->vertical = o == B_VERTICAL;

	b->controls = new vector<struct boxchild>();

	uiHaikuFinishNewControl(b, uiBox);
	uiControl(b)->ContainerUpdateState = boxContainerUpdateState;

	return b;
}

uiBox *uiNewHorizontalBox(void)
{
	return finishNewBox(B_HORIZONTAL);
}

uiBox *uiNewVerticalBox(void)
{
	return finishNewBox(B_VERTICAL);
}
