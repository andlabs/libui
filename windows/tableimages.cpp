// 10 june 2018
#include "uipriv_windows.hpp"
#include "table.hpp"

/*
This file handles both images and checkboxes in tables.

For images, we'll do a similar thing to what text columns do: cycle out images from the small image list every few LVN_GETDISPINFO notifications.

For checkboxes, the native list view checkbox functionality uses state images, but those are only supported on the main item, not on subitems. So instead, we'll do them on normal images instead.
TODO will this affect accessibility?

We'll use the small image list. For this, the first few items will be reserved for checkboxes, and the last few for cell images.
*/

// checkboxes TODOs:
// - see if we need to get rid of the extra margin in subitems
// - get rid of the extra bitmap margin space before text
// - get rid of extra whitespace before text on subitems (this might not be necessary if we can fill the background of images AND this amount is the same as on the first column; it is a hardcoded 2 logical units in the real list view code)

#define nCheckboxImages 4

HRESULT uiprivLVN_GETDISPINFOImagesCheckboxes(uiTable *t, NMLVDISPINFOW *nm, uiprivTableColumnParams *p)
{
	uiTableData *data;
	HRESULT hr;

	if (nm->item.iSubItem == 0 && p->imageModelColumn == -1 && p->checkboxModelColumn == -1) {
		// having an image list always leaves space for an image on the main item :|
		// other places on the internet imply that you should be able to do this but that it shouldn't work
		// but it works perfectly (and pixel-perfectly too) for me, so...
		nm->item.mask |= LVIF_INDENT;
		nm->item.iIndent = -1;
	}
	if ((nm->item.mask & LVIF_IMAGE) == 0)
		return S_OK;		// nothing to do here

	// TODO
	nm->item.iImage = -1;
	if (p->imageModelColumn != -1 || p->checkboxModelColumn != -1)
		nm->item.iImage = 0;
	return S_OK;

	if (p->imageModelColumn != -1) {
		nm->item.iImage = 0;
		return S_OK;
	}

	if (p->checkboxModelColumn != -1) {
#if 0
		// TODO handle enabled
		data = (*(t->model->mh->CellValue))(t->model->mh, t->model, nm->item.iItem, p->textModelColumn);
		checked = uiTableDataInt(data) != 0;
		uiFreeTableData(data);
		nm->item.iImage = 0;
		if (checked)
			nm->item.iImage = 1;
		nm->item.mask |= LVIF_IMAGE;
#endif
		nm->item.mask |= LVIF_IMAGE;
		nm->item.iImage = nm->item.iItem % 4;
		return S_OK;
	}

	// TODO see if this is correct
	nm->item.iImage = -1;
	return S_OK;
}
