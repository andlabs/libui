// 23 june 2018

#ifdef __cplusplus
extern "C" {
#endif

// tablemodel.c
extern uiTableModelHandler *uiprivTableModelHandler(uiTableModel *m);
extern int uiprivTableModelNumColumns(uiTableModel *m);
extern uiTableValueType uiprivTableModelColumnType(uiTableModel *m, int column);
extern int uiprivTableModelNumRows(uiTableModel *m);
extern uiTableValue *uiprivTableModelCellValue(uiTableModel *m, int row, int column);
extern void uiprivTableModelSetCellValue(uiTableModel *m, int row, int column, const uiTableValue *value);

#ifdef __cplusplus
}
#endif
