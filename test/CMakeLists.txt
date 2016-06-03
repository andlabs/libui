# 3 june 2016

if(WIN32)
	set(_TEST_RESOURCES_RC resources.rc)
endif()

_add_exec(tester
	drawtests.c
	main.c
	menus.c
	page1.c
	page10.c
	page11.c
	page12.c
	page13.c
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
	spaced.c
	${_TEST_RESOURCES_RC}
)
target_include_directories(tester
	PRIVATE test
)
set_target_properties(tester PROPERTIES
	OUTPUT_NAME test
	WIN32_EXECUTABLE FALSE
)
