# 23 march 2019

windows = import('windows')

libui_sources += [
	'windows/alloc.cpp',
	'windows/area.cpp',
	'windows/areadraw.cpp',
	'windows/areaevents.cpp',
	'windows/areascroll.cpp',
	'windows/areautil.cpp',
	'windows/attrstr.cpp',
	'windows/box.cpp',
	'windows/button.cpp',
	'windows/checkbox.cpp',
	'windows/colorbutton.cpp',
	'windows/colordialog.cpp',
	'windows/combobox.cpp',
	'windows/container.cpp',
	'windows/control.cpp',
	'windows/d2dscratch.cpp',
	'windows/datetimepicker.cpp',
	'windows/debug.cpp',
	'windows/draw.cpp',
	'windows/drawmatrix.cpp',
	'windows/drawpath.cpp',
	'windows/drawtext.cpp',
	'windows/dwrite.cpp',
	'windows/editablecombo.cpp',
	'windows/entry.cpp',
	'windows/events.cpp',
	'windows/fontbutton.cpp',
	'windows/fontdialog.cpp',
	'windows/fontmatch.cpp',
	'windows/form.cpp',
	'windows/graphemes.cpp',
	'windows/grid.cpp',
	'windows/group.cpp',
	'windows/image.cpp',
	'windows/init.cpp',
	'windows/label.cpp',
	'windows/main.cpp',
	'windows/menu.cpp',
	'windows/multilineentry.cpp',
	'windows/opentype.cpp',
	'windows/parent.cpp',
	'windows/progressbar.cpp',
	'windows/radiobuttons.cpp',
	'windows/separator.cpp',
	'windows/sizing.cpp',
	'windows/slider.cpp',
	'windows/spinbox.cpp',
	'windows/stddialogs.cpp',
	'windows/tab.cpp',
	'windows/table.cpp',
	'windows/tabledispinfo.cpp',
	'windows/tabledraw.cpp',
	'windows/tableediting.cpp',
	'windows/tablemetrics.cpp',
	'windows/tabpage.cpp',
	'windows/text.cpp',
	'windows/utf16.cpp',
	'windows/utilwin.cpp',
	'windows/window.cpp',
	'windows/winpublic.cpp',
	'windows/winutil.cpp',
]

# resources.rc only contains the libui manifest.
# For a DLL, we have to include this directly, so we do so.
# Windows won't link resources in static libraries, so including this would have no effect.
# In those cases, we just need them to include the manifest with the executable (or link it directly into the output executable themselves); they can also customize the manifest as they see fit (assuming nothing breaks in the process).
if libui_mode == 'shared'
	libui_sources += [
		windows.compile_resources('resources.rc',
			args: libui_manifest_args,
			depend_files: ['libui.manifest']),
	]
endif

# TODO prune this list
foreach lib : ['user32', 'kernel32', 'gdi32', 'comctl32', 'uxtheme', 'msimg32', 'comdlg32', 'd2d1', 'dwrite', 'ole32', 'oleaut32', 'oleacc', 'uuid', 'windowscodecs']
	libui_deps += [
		meson.get_compiler('cpp').find_library(lib,
			required: true),
	]
endforeach

if libui_OS == 'windows' and libui_mode == 'shared' and not libui_MSVC
	error('Sorry, but libui for Windows can currently only be built as a static library with MinGW. You will need to either build as a static library or switch to MSVC.')
endif
