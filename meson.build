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
libui_soversion = ''
libui_rpath = ''
if libui_OS == 'windows'
	# TODO c_winlibs and cpp_winlibs are the windows libraries
	#	user32 kernel32 gdi32 comctl32 uxtheme msimg32 comdlg32 d2d1 dwrite ole32 oleaut32 oleacc uuid windowscodecs
elif libui_OS == 'darwin'
	# TODO libraries
	# "-framework Foundation" "-framework AppKit"
	libui_soversion = 'A'
	# the / is required by some older versions of OS X
	libui_rpath = '@executable_path/'
	# TODO min version 10.8
else
	gtk = dependency('gtk+-3.0',
		version: '>=3.10.0',
		required: true)
	libui_deps += [gtk]
	libui_soversion = '0'
	libui_rpath = '$ORIGIN'
endif

if libui_MSVC
	# TODO subsystem version

	# TODO -Wno-switch equivalent
	# TODO /sdl turns C4996 into an ERROR
	# don't use /analyze; that requires us to write annotations everywhere
	# TODO undecided flags from qo?
	# the RTC flags are only supplied in debug builds because they are only supposed to be used by debug builds (see "This is because run-time error checks are not valid in a release (optimized) build." in https://docs.microsoft.com/cpp/build/reference/rtc-run-time-error-checks)
	# /RTCc is not supplied because it's discouraged as of VS2015; see https://www.reddit.com/r/cpp/comments/46mhne/rtcc_rejects_conformant_code_with_visual_c_2015/d06auq5
	# /EHsc is to shut the compiler up in some cases
	# TODO add these compiler flags (assuming meson doesn't provide an alternate method for these, which I know it does for EHsc):
	#	/wd4100 (equivalent of -Wno-unused-parameter)
	#	/bigobj /nologo
	#	$<$<CONFIG:Debug>:/RTC1 /RTCs /RTCu>
	#	/EHsc

	# TODO add these linker flags (for each: maybe? depends on whether meson does this itself)
	#	/LARGEADDRESSAWARE
	#	/NOLOGO
	#	/INCREMENTAL:NO
	#	/MANIFEST:NO
else
	# TODO add these C/C++ compiler options
	#	-Wno-unused-parameter
	#	-Wno-switch

	if libui_OS == 'windows'
		# don't require shipping the MinGW-w64 DLLs
		# TODO add these to the linker options
		#	-static
		#	-static-libgcc
		#	-static-libstdc++
	endif
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
	gnu_symbol_visibility: 'hidden',
	soversion: libui_soversion)
install_headers('ui.h')

# TODO test
# TODO examples
