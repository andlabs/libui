OSMFILES = \
	alloc.m \
	button.m \
	checkbox.m \
	entry.m \
	init.m \
	label.m \
	main.m \
	newcontrol.m \
	parent.m \
	tab.m \
	text.m \
	util.m \
	window.m

xCFLAGS += -mmacosx-version-min=10.7 -DMACOSX_DEPLOYMENT_TARGET=10.7
xLDFLAGS += -mmacosx-version-min=10.7 -lobjc -framework Foundation -framework AppKit

OUT = new
