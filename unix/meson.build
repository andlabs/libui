# 23 march 2019

libui_sources += [
	'unix/main.c',
]

libui_deps += [
	dependency('gtk+-3.0',
		version: '>=3.10.0',
		method: 'pkg-config',
		required: true),
	# We specify these as not required because some Unix systems include them with libc instead of providing them as separate files (thanks textshell and jpakkane in freenode #mesonbuild)
	meson.get_compiler('c').find_library('m',
		required: false),
	meson.get_compiler('c').find_library('dl',
		required: false),
]
libui_soversion = '0'
libui_rpath = '$ORIGIN'
