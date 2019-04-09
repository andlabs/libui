# 23 march 2019

libui_test_sources = [
	'drawtests.c',
	'images.c',
	'main.c',
	'menus.c',
	'page1.c',
	'page2.c',
	'page3.c',
	'page4.c',
	'page5.c',
	'page6.c',
	'page7.c',
	'page7a.c',
	'page7b.c',
	'page7c.c',
#	'page8.c',
#	'page9.c',
#	'page10.c',
	'page11.c',
	'page12.c',
	'page13.c',
	'page14.c',
	'page15.c',
	'page16.c',
	'spaced.c',
]

if libui_OS == 'windows'
	libui_test_manifest = 'test.manifest'
	if libui_mode == 'static'
		libui_test_manifest = 'test.static.manifest'
	endif
	libui_test_sources += [
		windows.compile_resources('resources.rc',
			args: libui_manifest_args,
			depend_files: [libui_test_manifest]),
	]
endif

# TODO meson doesn't let us name this target test, but also doesn't seem to provide a way to override the executable name???? we'll probably need to file a feature request for this
# TODO once we upgrade to 0.49.0, add pie: true
executable('tester', libui_test_sources,
	dependencies: libui_binary_deps,
	link_with: libui_libui,
	gui_app: false,
	install: false)
