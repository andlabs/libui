# 23 march 2019

# TODO
#if(WIN32)
#	set(_TEST_RESOURCES_RC resources.rc)
#endif()

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

# TODO meson doesn't let us name this target test, but also doesn't seem to provide a way to override the executable name????
executable('tester', libui_test_sources,
	include_directories: libui_base_include_directories + [include_directories('.')],
	dependencies: libui_binary_deps,
	link_with: libui_libui,
	build_by_default: false,
	gui_app: false,
	install: false)
