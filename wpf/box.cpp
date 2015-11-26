// 26 november 2015
#include "uipriv_wpf.hpp"

using namespace System::Collections::Generic;

// TODO
// - save Alignment of children?
// - SnapsToDevicePixels? http://stackoverflow.com/questions/10718985/grid-border-gap-between-cells

ref class boxChild {
public:
	uiControl *c;
	Border ^border;
	int stretchy;
};

struct uiBox {
	uiWindowsControl c;
	// we could've used StackPanel but that doesn't care about available size
	gcroot<Grid ^> *grid;
	gcroot<List<boxChild ^> ^> *children;
	int vertical;
	int padded;
};

static void onDestroy(uiBox *b);

uiWindowsDefineControlWithOnDestroy(
	uiBox,								// type name
	uiBoxType,							// type function
	grid,									// handle
	onDestroy(hthis);						// on destroy
)

static void onDestroy(uiBox *b)
{
	List<boxChild ^> ^children;

	children = *(b->children);
	while (children->Count != 0) {
		children[0]->border->Child = nullptr;
		uiControlSetParent(children[0]->c, NULL);
		uiControlDestroy(children[0]->c);
		children->RemoveAt(0);
	}
	delete b->children;
}

static void boxContainerUpdateState(uiControl *c)
{
	uiBox *b = uiBox(c);
	List<boxChild ^> ^children;
	int i;

	children = *(b->children);
	for (i = 0; i < children->Count; i++)
		controlUpdateState(children[i]->c);
}

// Grid unfortunately does not have a way to set the spacing between rows and columns.
// This means we have to do padding ourselves.
static void resetMargins(uiBox *b)
{
	double paddingUnit;
	Thickness first;
	Thickness after;
	List<boxChild ^> ^children;
	int i;

	children = *(b->children);
	if (children->Count == 0)
		return;

	paddingUnit = 0;
	if (b->padded)
		paddingUnit = 5;
	first = Thickness(0, 0, 0, 0);
	after = Thickness(paddingUnit, 0, 0, 0);
	if (b->vertical)
		after = Thickness(0, paddingUnit, 0, 0);

	// TODO padding?
	children[0]->border->Margin = first;
	for (i = 1; i < children->Count; i++)
		children[i]->border->Margin = after;
}

void uiBoxAppend(uiBox *b, uiControl *c, int stretchy)
{
	Grid ^g;
	boxChild ^bc;
	int pos;

	bc = gcnew boxChild();
	bc->c = c;
	bc->border = gcnew Border();
	bc->stretchy = stretchy;

	bc->border->Child = genericHandle(bc->c);

	g = *(b->grid);

	// get position before adding the child so that we get the right count value
	pos = g->ColumnDefinitions->Count;
	if (b->vertical)
		pos = g->RowDefinitions->Count;

	g->Children->Add(bc->border);

	if (b->vertical) {
		g->SetRow(bc->border, pos);
		g->SetColumn(bc->border, 0);
		// apparently we have to do this ourselves...
		g->RowDefinitions->Add(gcnew RowDefinition());
		if (bc->stretchy)
			g->RowDefinitions[pos]->Height = GridLength(1, GridUnitType::Star);
		else
			g->RowDefinitions[pos]->Height = GridLength(1, GridUnitType::Auto);
	} else {
		g->SetRow(bc->border, 0);
		g->SetColumn(bc->border, pos);
		g->ColumnDefinitions->Add(gcnew ColumnDefinition());
		if (bc->stretchy)
			g->ColumnDefinitions[pos]->Width = GridLength(1, GridUnitType::Star);
		else
			g->ColumnDefinitions[pos]->Width = GridLength(1, GridUnitType::Auto);
	}

	uiControlSetParent(bc->c, uiControl(b));
	(*(b->children))->Add(bc);
	resetMargins(b);
}

void uiBoxDelete(uiBox *b, uintmax_t index)
{
	boxChild ^bc;
	List<boxChild ^> ^children;

	children = *(b->children);
	bc = children[index];
	children->RemoveAt(index);
	uiControlSetParent(bc->c, NULL);
	bc->border->Child = nullptr;
	(*(b->grid))->Children->RemoveAt(index);
	resetMargins(b);
}

int uiBoxPadded(uiBox *b)
{
	return b->padded;
}

void uiBoxSetPadded(uiBox *b, int padded)
{
	b->padded = padded;
	resetMargins(b);
}

static uiBox *finishNewBox(int vertical)
{
	uiBox *b;

	b = (uiBox *) uiNewControl(uiBoxType());

	b->grid = new gcroot<Grid ^>();
	*(b->grid) = gcnew Grid();

	b->vertical = vertical;
	if (b->vertical) {
		(*(b->grid))->ColumnDefinitions->Add(gcnew ColumnDefinition());
		(*(b->grid))->ColumnDefinitions[0]->Width = GridLength(1, GridUnitType::Star);
	} else {
		(*(b->grid))->RowDefinitions->Add(gcnew RowDefinition());
		(*(b->grid))->RowDefinitions[0]->Height = GridLength(1, GridUnitType::Star);
	}

	b->children = new gcroot<List<boxChild ^> ^>();
	*(b->children) = gcnew List<boxChild ^>();

	uiWindowsFinishNewControl(b, uiBox, grid);
	uiControl(b)->ContainerUpdateState = boxContainerUpdateState;

	return b;
}

uiBox *uiNewHorizontalBox(void)
{
	return finishNewBox(0);
}

uiBox *uiNewVerticalBox(void)
{
	return finishNewBox(1);
}
