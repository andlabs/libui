
set(COMMON_C_FILES
	common/areaevents.c
	common/control.c
	common/matrix.c
	common/menu.c
	common/ptrarray.c
	common/shouldquit.c
	common/types.c
)

set(COMMON_H_FILES
	common/uipriv.h
)


set(COMMON_FILES ${COMMON_C_FILES} ${COMMON_H_FILES} PARENT_SCOPE)
