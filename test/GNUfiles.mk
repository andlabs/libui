# 22 april 2015

CFILES += \
	test/drawtests.c \
	test/main.c \
	test/menus.c \
	test/page1.c \
	test/page2.c \
	test/page3.c \
	test/page4.c \
	test/page5.c \
	test/page6.c \
	test/page7.c \
	test/page7a.c \
	test/page7b.c \
	test/page7c.c \
	test/page8.c \
	test/page9.c \
	test/spaced.c

HFILES += \
	test/test.h

ifeq ($(OS),windows)
RCFILES += \
	test/resources.rc
endif
