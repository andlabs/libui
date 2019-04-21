# 23 march 2019

libui_sources += [
	'darwin/main.m',
]

libui_deps += [
	meson.get_compiler('objc').find_library('objc',
		required: true),
	dependency('appleframeworks',
		modules: ['Foundation', 'AppKit'],
		required: true),
]
libui_soversion = 'A'
# the / is required by some older versions of OS X
libui_rpath = '@executable_path/'
