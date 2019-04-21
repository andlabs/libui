# 23 march 2019

windows = import('windows')

libui_sources += [
	'windows/init.cpp',
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

foreach lib : ['user32', 'kernel32', 'gdi32', 'comctl32']
	libui_deps += [
		meson.get_compiler('cpp').find_library(lib,
			required: true),
	]
endforeach

if libui_OS == 'windows' and libui_mode == 'shared' and not libui_MSVC
	error('Sorry, but libui for Windows can currently only be built as a static library with MinGW. You will need to either build as a static library or switch to MSVC.')
endif
