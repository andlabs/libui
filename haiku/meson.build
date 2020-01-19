# 12 january 2020

libui_sources += [
	'haiku/controls.cpp',
	'haiku/main.cpp',
]

libui_deps += [
	meson.get_compiler('cpp').find_library('root',
		required: true),
	meson.get_compiler('cpp').find_library('be',
		required: true),
]
libui_soversion = '0'
# TODO possible meson bug? Haiku shouldn't have an rpath, right?...
