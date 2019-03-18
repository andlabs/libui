# 17 march 2019

libui_OS = target_machine.system()
libui_MSVC = meson.get_compiler('c').get_id() == 'msvc'

libui_languages = ['c', 'c++']
if libui_OS == 'darwin'
	libui_languages += ['objc']
endif

libui_default_options = [
	'default_library=shared',			# build shared libraries by default
]
libui_forced_options = {
	'warning_level':	'3',			# always max warnings
	'b_pch':			'false',		# we don't want precompiled headers
	'b_staticpic':		'true',		# use PIC even for static libraries
	'c_std':			'c99',		# strict C99
	'cpp_std':			'c++11',		# strict C++11
}
foreach name, value : libui_forced_options
	libui_default_options += [name + '=' + value]
endforeach

project('libui', libui_languages,
	default_options: libui_default_options,
	license: 'MIT')

foreach name, value : libui_forced_options
	actual = '@0@'.format(get_option(name))
	if actual != value
		error('sorry, but libui requires that option ' + name + ' has the default value ' + value)
	endif
endforeach

libui_mode = get_option('default_library')
if libui_mode == 'both'
	error('sorry, but libui does not support building both shared and static libraries at the same time, because Windows resource file rules differ between the two')
endif
if libui_OS == 'windows' and libui_mode == 'shared' and not libui_MSVC
	error('sorry, but libui currently does not support building a shared library with MinGW; use -Ddefault_library=static instead')
endif

libui_deps = []
libui_libversion = ''
libui_rpath = ''
if libui_OS == 'windows'
	# TODO c_winlibs and cpp_winlibs are the windows libraries
elif libui_OS == 'darwin'
	# TODO libraries
	libui_libversion = 'A'
	# the / is required by some older versions of OS X
	libui_rpath = '@executable_path/'
	# TODO min version 10.8
else
	gtk = dependency('gtk+-3.0',
		version: '>=3.10.0',
		required: true)
	libui_deps += [gtk]
	libui_libversion = '0'
	libui_rpath = '$ORIGIN'
endif

libui_sources = []
libui_extra_libs = []
subdir('common')
if libui_OS == 'windows'
	subdir('windows')
	install_headers('ui_windows.h')
elif libui_OS == 'darwin'
	subdir('darwin')
	install_headers('ui_darwin.h')
else
	subdir('unix')
	install_headers('ui_unix.h')
endif
libui_libui = library('ui', libui_sources,
	dependencies: libui_deps,
	build_rpath: libui_rpath,
	install_rpath: libui_rpath,
	install: true,
	gnu_symbol_visibility: 'hidden')
install_headers('ui.h')

# TODO test
# TODO examples
