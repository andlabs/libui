# 23 march 2019

libui_test_sources += [
	'lib/testing.c',
	'lib/testingpriv.c',
	'lib/timer.c',
]

if libui_OS == 'windows'
	libui_test_sources += [
		'lib/thread_windows.c',
		'lib/timer_windows.c',
	]
else
	libui_test_sources += [
		'lib/thread_darwinunix.c',
		'lib/timer_darwinunix.c'
	]
endif

libui_test_deps += [
	dependency('threads',
		required: true),
]
if libui_OS == 'windows'
	# static mode already gives us these dependencies
	if libui_mode != 'static'
		libui_test_deps += [
			meson.get_compiler('c').find_library('kernel32',
				required: true),
			meson.get_compiler('c').find_library('user32',
				required: true),
		]
	endif
endif
