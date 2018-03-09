# 3 june 2016

list(APPEND _LIBUI_SOURCES
	common/attribute.c
	common/attrlist.c
	common/attrstr.c
	common/areaevents.c
	common/control.c
	common/debug.c
#	common/drawtext.c
	common/matrix.c
	common/opentype.c
	common/shouldquit.c
	common/userbugs.c
	common/utf.c
)
set(_LIBUI_SOURCES ${_LIBUI_SOURCES} PARENT_SCOPE)

list(APPEND _LIBUI_INCLUDEDIRS
	common
)
set(_LIBUI_INCLUDEDIRS ${_LIBUI_INCLUDEDIRS} PARENT_SCOPE)
