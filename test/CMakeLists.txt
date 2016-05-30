
add_executable(tester MACOSX_BUNDLE
    test.h
	drawtests.c
	main.c
	menus.c
	page1.c
	page2.c
	page3.c
	page4.c
	page5.c
	page6.c
	page7.c
	page7a.c
	page7b.c
	page7c.c
	page8.c
	page9.c
	page10.c
	page11.c
	page12.c
	page13.c
	spaced.c
)

use_libui(tester)
