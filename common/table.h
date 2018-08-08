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
extern const uiTableTextColumnOptionalParams uiprivDefaultTextColumnOptionalParams;
extern int uiprivTableModelCellEditable(uiTableModel *m, int row, int column);
extern int uiprivTableModelColorIfProvided(uiTableModel *m, int row, int column, double *r, double *g, double *b, double *a);

#ifdef __cplusplus
}
#endif
