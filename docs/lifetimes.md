# Object lifetimes

Know them.

One of the more annoying parts of libui is that it has to juggle an OS control handle and at least one C data structure for a single uiControl, uiParent, uiWindow, etc. To that end, this document exists to lay the ground rules for object lifetimes.

## To end programmers

TODO complain loudly if rules broken
TODO complain loudly if Destroying a control with a parent

## Implementation

Note that control implementations generally don't need to worry about backend-specific information; these are handled by the backends's convenience functions. If you bypass those, however, you *will* need to worry.

### Backend-Agnostic (uiBox, uiGrid)

### Windows

TODO write a lifetimes.c

### Unix

see os x section
TODO write a lifetimes.c

### Mac OS X

notes about not monitoring references or destruction here
do the whole "keep a reference around" thing
TODO drop the deleted controls view
