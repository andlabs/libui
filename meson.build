# 17 march 2019

# Here's the overall idea: the build syntax would be something like
# $ meson setup build \
#     [--buildtype=debug|release] \        (core option, default debug)
#     [--default_library=static|shared] \  (core option, default shared)
#     [--b_sanitize=whatever]
# It turns out that I wouldn't really need any custom options; go figure.
# I'm not sure how to allow building 32-bit instead of 64-bit with meson.
# I'd also like all the outputs to appear in a folder out/ in the build folder.
# I'd also like to be able to omit the library's dependency libraries in the executables if building a shared library, as those are not required in that case; I'd need to require them in a static library.
# And for Windows, the shared library (but not shared executables) needs an additional resource file, and the static executables (but not the static library) need a *different* resource file. (Windows static libraries can't contain resources anyway; the linkers will just ignore the resource objects.)

# we have to specify C++ here even if we're not using C++ immediately because otherwise none of the C++ built-in options are available (sigh; TODO)
project('libui', ['c', 'cpp'],
	meson_version: '>=0.48.0',
	default_options: [
		'buildtype=debug',				# build debug by default
		'default_library=shared',			# build shared libraries by default

		# these are forced options, but meson doesn't let me set these up separately before I call project() (TODO)
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

# TODO move these below the next part (I need them here for other reasons)
libui_OS = host_machine.system()
libui_MSVC = meson.get_compiler('c').get_id() == 'msvc'

if libui_OS == 'darwin'
	add_languages('objc',
		required: true)
endif

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
	# TODO why does meson need this guard? why aren't all options defined regardless of target?!
	if not ((name == 'c_winlibs' or name == 'cpp_eh' or name == 'cpp_winlibs') and not libui_MSVC) and not (name == 'c_std' and libui_MSVC)
		actual = '@0@'.format(get_option(name))
		if actual != value
			error('sorry, but libui requires that option ' + name + ' has the default value ' + value)
		endif
	endif
endforeach

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

libui_manifest_args = []
if libui_mode == 'static'
	libui_project_compile_args += ['-D_UI_STATIC']
	libui_manifest_args = ['-D_UI_STATIC']
endif

add_project_arguments(libui_project_compile_args,
	language: ['c', 'cpp', 'objc'])
add_project_link_arguments(libui_project_link_args,
	language: ['c', 'cpp', 'objc'])

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
