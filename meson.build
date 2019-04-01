# 17 march 2019

# TODO I'm not sure how to allow building 32-bit instead of 64-bit with meson

# TODO remove cpp from this list once https://github.com/mesonbuild/meson/issues/5181 is settled; move it to the OS checks and under cpp-multithread
project('libui', ['c', 'cpp'],
	meson_version: '>=0.48.0',
	default_options: [
		'buildtype=debug',				# build debug by default
		'default_library=shared',			# build shared libraries by default
		'layout=flat',					# keep all outputs together by default

		# these are forced options, but meson doesn't let me set these up separately before I call project() (TODO https://github.com/mesonbuild/meson/issues/5179)
		'warning_level=3',				# always max warnings
		'b_pch=false',					# we don't want precompiled headers
		'b_staticpic=true',				# use PIC even for static libraries
		'c_std=c99',					# strict C99
		'c_winlibs=',					# we define our own Windows libraries
		'cpp_std=c++11',				# strict C++11
		'cpp_eh=sc',					# shut the compiler up in some cases
		'cpp_winlibs=',					# likewise as with c_winlibs
	],
	license: 'MIT')

# TODO after https://github.com/mesonbuild/meson/issues/5179 is settled, remove these
libui_OS = host_machine.system()
libui_MSVC = meson.get_compiler('c').get_id() == 'msvc'

# TODO switch to tabs; the spaces are just so I can share this file while I'm writing it
libui_forced_options = {
	'warning_level': '3',     # always max warnings
	'b_pch':         'false', # we don't want precompiled headers
	'b_staticpic':   'true',  # use PIC even for static libraries
	'c_std':         'c99',   # strict C99
	'c_winlibs':     '[]',    # we define our own Windows libraries
	'cpp_std':       'c++11', # strict C++11
	'cpp_eh':        'sc',    # shut the compiler up in some cases
	'cpp_winlibs':   '[]',    # likewise as with c_winlibs
}
foreach name, value : libui_forced_options
	# TODO rewrite this when https://github.com/mesonbuild/meson/issues/5181 is settled
	if not ((name == 'c_winlibs' or name == 'cpp_eh' or name == 'cpp_winlibs') and not libui_MSVC) and not (name == 'c_std' and libui_MSVC)
		actual = '@0@'.format(get_option(name))
		if actual != value
			error('sorry, but libui requires that option ' + name + ' has the default value ' + value)
		endif
	endif
endforeach

libui_OS = host_machine.system()
libui_MSVC = meson.get_compiler('c').get_id() == 'msvc'

if libui_OS == 'darwin'
	add_languages('objc',
		required: true)
endif

libui_mode = get_option('default_library')
if libui_mode == 'both'
	error('sorry, but libui does not support building both shared and static libraries at the same time, because Windows resource file rules differ between the two')
endif

libui_is_debug = get_option('buildtype').startswith('debug')

libui_project_compile_args = []
libui_project_link_args = []

if libui_OS == 'darwin'
	libui_macosx_version_min = '-mmacosx-version-min=10.8'
	libui_project_compile_args += [libui_macosx_version_min]
	libui_project_link_args += [libui_macosx_version_min]
endif

if libui_MSVC
	# TODO subsystem version

	libui_project_compile_args += [
		'/wd4100',
		'/bigobj',
	]
	if libui_is_debug
		libui_project_compile_args += ['/RTC1', '/RTCs', '/RTCu']
	endif

	libui_project_link_args += [
		'/LARGEADDRESSAWARE',
		'/INCREMENTAL:NO',
		'/MANIFEST:NO',
	]

	# TODO autogenerate a .def file?
else
	libui_project_compile_args += [
		'-Wno-unused-parameter',
		'-Wno-switch',
	]

	if libui_OS == 'windows'
		# don't require shipping the MinGW-w64 DLLs
		libui_project_link_args += [
			'-static',
			'-static-libgcc',
			'-static-libstdc++',
		]
	endif
endif

# TODO come up with a better way to do this, both for libui (the compiler define, used by winapi.hpp, and the manifest args) and for the binaries (the manifest args)
# TODO (after the above TODO is resolved) move that below the part below that actually adds these arguments
libui_manifest_args = []
if libui_mode == 'static'
	libui_project_compile_args += ['-D_UI_STATIC']
	libui_manifest_args = ['-D_UI_STATIC']
endif

add_project_arguments(libui_project_compile_args,
	language: ['c', 'cpp', 'objc'])
add_project_link_arguments(libui_project_link_args,
	language: ['c', 'cpp', 'objc'])

# TODO:
# meson determins whether -Wl,--no-undefined is provided via
# built-in option b_lundef, and it's true by default, which is what
# we want (so I don't make mistakes like asking for unknown
# functions in my dependencies). However, meson also is smart
# about specifying this properly on systems that don't support it, like
# FreeBSD (where I had the comment "figure out why FreeBSD
# follows linked libraries here" when I was on cmake) and OpenBSD
# (according to someone on freenode #mesonbuild), but it isn't clear
# whether it's just ignored or if the value is forced to false.
# Therefore, once this is determined, we can uncomment the
# following.
libui_libui_link_args = []
#if not libui_MSVC and get_option("b_lundef")
	# TODO what should this be on MSVC?
#	libui_libui_link_args += ['-Wl,--no-allow-shlib-undefined']
#endif

libui_sources = []
libui_deps = []
libui_soversion = ''
libui_rpath = ''
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
	name_prefix: 'lib',		# always call it libui, even in Windows DLLs
	install: true,
	gnu_symbol_visibility: 'hidden',
	c_args: ['-Dlibui_EXPORTS'],
	cpp_args: ['-Dlibui_EXPORTS'],
	objc_args: ['-Dlibui_EXPORTS'],
	link_args: libui_libui_link_args,
	soversion: libui_soversion,
	darwin_versions: [])		# TODO
install_headers('ui.h')

# TODO when the API is stable enough to be versioned, create a pkg-config file (https://mesonbuild.com/Pkgconfig-module.html) and a declare_dependency() section too

libui_binary_deps = []
if libui_mode == 'static'
	libui_binary_deps = libui_deps
endif
subdir('test')
subdir('examples')
