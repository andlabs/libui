# Thoughts on table control support

(BenC - 2017-09-25)

## win32 comctl listview TODOs

- maintain proper list of column parts, even if dummy.
   (or cut down the public API to text columns only for now
   until a better windows table control is available - see below
   for more thoughts on this)
- support editing of cells
- expandable parts (allocate leftover space)
- how to determine column width? random sample of data?


## Is the parts system the right abstraction?

Some toolkits support implementing cell views as custom
widget layouts eg [item delgates in QML](http://doc.qt.io/qt-5/qml-qtquick-controls-tableview.html#itemDelegate-prop).
So an item can be just about any widget layout you want.

Is that kind of thing a future aspiration for libui?
If so, I'd suggest cutting the parts API back to just
uiTableAppendTextColumn() for now.

The parts API gives _some_ of the flexiblity, but would likely
get in the way if libui went the whole hog on custom item view
layouts.


## API change: support bulk invalidation of model

Currently, the only way to notify the control that new data has
been added is via the uiTableModelRow[Inserted|Changed|Deleted]
functions. This inevitably causes redraws for each item, which
can get _really_ slow for large numbers of items.

The use cases are:
 - adding large numbers of rows to existing models
 - sorting (eg clicking on various column headings to change
   the sort field and order.
   The user-supplied model needs to handle the sorting, the
   table control can't help.

I think a single uiTableModelAllChanged() function would be
enough. The underlying control needs to know that it should
to redraw everything visible (and the number of items might
have chnaged).


## API change: support flags/styles

- single or multiselect
- hide column headers?
- allow column reordering?
- allow column resizing?
- allow click on columns (+indicators for ascending/descending)?
  (will require corresponding event handling/callbacks)

I'd suggest implementing whichever of these flags are supported on
all platforms.




## API Change: Selection

In general, most toolkits avoid exposing a flat array of
selected items, presumably because the dataset could be
very large. Usually there are methods to iterate over
selection, often with simplified interface for
single-selection-only tables.

[GtkTreeView](https://developer.gnome.org/gtk3/stable/GtkTreeView.html)
- has a selection object for getting/setting selection
  - can fetch selection as a list, or via a foreach callback
  - need to map GtkTreeview paths back to row indexes

[QML TableView](http://doc.qt.io/qt-5/qml-qtquick-controls-tableview.html)
- has a selection object for getting/setting selection
- supports iteration of selected items

[OSX NSTableView](https://developer.apple.com/documentation/appkit/nstableview)
- has an IndexSet to hold selected items
   - general purpose class? Lots of general set methods

win32 commonctrl listview
- items have LVIS_SELECTED flag
- state changes notified via
  [LVN_ODSTATECHANGED](https://msdn.microsoft.com/en-us/library/windows/desktop/bb774859(v=vs.85).aspx)
  (indicates one or more contiguous items have changed state)
- Use LVN_GETNEXTITEM to iterate through items with selected state

[wxWidgets wxDataViewCtrl](http://docs.wxwidgets.org/3.0/classwx_data_view_ctrl.html)
- provides `GetSelections( wxDataViewItemArray& sel)`, where `wxDataViewItemArray`
  is presumably a growable array of some kind.


aiming to support:

- a "selection has changed" event
    `uiTableOnSelectionChanged(uiTable* t, handlerfn fn, void* userdata)`?
- functions to programatically select/unselect item(s)
- access current selection (preferably by iteration rather than returning
  possibly-huge arrays)

