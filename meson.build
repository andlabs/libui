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
# and same for Objective-C because of add_project_arguments() (double sigh; TODO)
project('libui', ['c', 'cpp'],
	default_options: [
		'buildtype=debug',				# build debug by default
		'default_library=shared',			# build shared libraries by default

		# these are forced options, but meson doesn't let me set these up separately before I call project() (TODO)
		'warning_level=3',				# always max warnings
		'b_pch=false',					# we don't want precompiled headers
		'b_staticpic=true',				# use PIC even for static libraries
		'c_std=c99',					# strict C99
		'cpp_std=c++11',				# strict C++11
		'cpp_eh=sc',					# shut the compiler up in some cases
	],
	license: 'MIT')

# TODO move these below the next part (I need them here for other reasons)
libui_OS = host_machine.system()
libui_MSVC = meson.get_compiler('c').get_id() == 'msvc'

if libui_OS == 'darwin'
	add_language('objc',
		required: true)
endif

# TODO switch to tabs; the spaces are just so I can share this file while I'm writing it
libui_forced_options = {
	'warning_level': '3',     # always max warnings
	'b_pch':         'false', # we don't want precompiled headers
	'b_staticpic':   'true',  # use PIC even for static libraries
	'c_std':         'c99',   # strict C99
	'cpp_std':       'c++11', # strict C++11
	'cpp_eh':        'sc',    # shut the compiler up in some cases
}
foreach name, value : libui_forced_options
	# TODO why does meson need this guard?
	if name != 'cpp_eh' or libui_MSVC
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
if libui_OS == 'windows' and libui_mode == 'shared' and not libui_MSVC
	error('sorry, but libui currently does not support building a shared library with MinGW; use --default_library=static instead')
endif

# TODO there has to be a better way to specify this
libui_buildtype = get_option('buildtype')
libui_is_debug = libui_buildtype == 'debug' or libui_buildtype == 'debugoptimized'

libui_deps = []
libui_soversion = ''
libui_rpath = ''
if libui_OS == 'windows'
	# TODO c_winlibs and cpp_winlibs are the windows libraries
	#	user32 kernel32 gdi32 comctl32 uxtheme msimg32 comdlg32 d2d1 dwrite ole32 oleaut32 oleacc uuid windowscodecs
elif libui_OS == 'darwin'
	libui_deps += [dependency('appleframeworks',
		modules: ['Foundation', 'AppKit'],
		required: true)]
	libui_soversion = 'A'
	# the / is required by some older versions of OS X
	libui_rpath = '@executable_path/'

	# TODO see if there's a more direct way to set this
	libui_macosx_version_min = '-mmacosx-version-min=10.8'
	add_project_arguments(libui_macosx_version_min,
		# TODO WHY IS THIS NEEDED MESON
		language: ['c', 'cpp', 'objc'])
	add_project_link_arguments(libui_macosx_version_min,
		language: ['c', 'cpp', 'objc'])
else
endif

# TODO see if there's a more direct way to set any of these
if libui_MSVC
	# TODO subsystem version

	# TODO is there a -Wno-switch equivalent?
	# TODO /sdl turns C4996 into an ERROR
	# don't use /analyze; that requires us to write annotations everywhere
	# TODO undecided flags from qo?
	# the RTC flags are only supplied in debug builds because they are only supposed to be used by debug builds (see "This is because run-time error checks are not valid in a release (optimized) build." in https://docs.microsoft.com/cpp/build/reference/rtc-run-time-error-checks)
	# /RTCc is not supplied because it's discouraged as of VS2015; see https://www.reddit.com/r/cpp/comments/46mhne/rtcc_rejects_conformant_code_with_visual_c_2015/d06auq5
	# TODO add these compiler flags (assuming meson doesn't provide an alternate method for these, which I know it does for EHsc):
	add_project_arguments(
		'/wd4100',
		'/bigobj', '/nologo',
		language: ['c', 'cpp', 'objc'])
	if libui_is_debug
		add_project_arguments('/RTC1', '/RTCs', '/RTCu',
			language: ['c', 'cpp', 'objc'])
	endif

	add_project_link_arguments(
		'/LARGEADDRESSAWARE',
		'/NOLOGO',
		'/INCREMENTAL:NO',
		'/MANIFEST:NO',
		language: ['c', 'cpp', 'objc'])
else
	add_project_arguments(
		'-Wno-unused-parameter',
		'-Wno-switch',
		language: ['c', 'cpp', 'objc'])

	if libui_OS == 'windows'
		# don't require shipping the MinGW-w64 DLLs
		add_project_link_arguments(
			'-static',
			'-static-libgcc',
			'-static-libstdc++',
			language: ['c', 'cpp', 'objc'])
	endif
endif

libui_sources = []
libui_base_include_directories = [include_directories('.')]
libui_include_directories = libui_base_include_directories
# TODO make this not +=
libui_deps += []
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
	include_directories: libui_include_directories,
	dependencies: libui_deps,
	build_rpath: libui_rpath,
	install_rpath: libui_rpath,
	install: true,
	gnu_symbol_visibility: 'hidden',
	c_args: ['-Dlibui_EXPORTS'],
	cpp_args: ['-Dlibui_EXPORTS'],
	objc_args: ['-Dlibui_EXPORTS'],
	soversion: libui_soversion,
	darwin_versions: [])		# TODO
install_headers('ui.h')

libui_binary_deps = []
if libui_mode == 'static'
	libui_binary_deps = libui_deps
endif
subdir('test')
# TODO examples
