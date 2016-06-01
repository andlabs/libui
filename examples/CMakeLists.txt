# 1 june 2016

include_directories(..)

_add_exec(controlgallery
	controlgallery/main.c
)
_add_exec(histogram
	histogram/main.c
)
_add_exec(cpp-multithread
	cpp-multithread/main.cpp
)

add_custom_target(examples
	DEPENDS
		controlgallery
		histogram
		cpp-multithread)
