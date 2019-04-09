// 3 june 2018
#import "../common/table.h"

// table.m
// TODO get rid of forward declaration
@class uiprivTableModel;
struct uiTableModel {
	uiTableModelHandler *mh;
	uiprivTableModel *m;
	NSMutableArray *tables;
};
struct uiTable {
	uiDarwinControl c;
	NSScrollView *sv;
	NSTableView *tv;
	uiprivScrollViewData *d;
	int backgroundColumn;
	uiTableModel *m;
};

// tablecolumn.m
@interface uiprivTableCellView : NSTableCellView
- (void)uiprivUpdate:(NSInteger)row;
@end
@interface uiprivTableColumn : NSTableColumn
- (uiprivTableCellView *)uiprivMakeCellView;
@end
