set(TEST_C_FILES
	test/drawtests.c
	test/main.c
	test/menus.c
	test/page1.c
	test/page2.c
	test/page3.c
	test/page4.c
	test/page5.c
	test/page6.c
	test/page7.c
	test/page7a.c
	test/page7b.c
	test/page7c.c
	test/page8.c
	test/page9.c
	test/spaced.c
)

set(TEST_H_FILES
	test/test.h
)


set(TEST_FILES ${TEST_C_FILES} ${TEST_H_FILES} PARENT_SCOPE)
