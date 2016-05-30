
macro(libui_example EXNAME EXSOURCE)
    add_executable(${EXNAME} MACOSX_BUNDLE ${EXSOURCE})
    use_libui(${EXNAME})    
endmacro()

set(CONTROLGALLERY_SOURCE controlgallery/main.c)
libui_example(controlgallery ${CONTROLGALLERY_SOURCE})

set(CPPMT_SOURCE cpp-multithread/main.cpp)
libui_example(cpp-multithread ${CPPMT_SOURCE})

set(HISTOGRAM_SOURCE histogram/main.c)
libui_example(histogram ${HISTOGRAM_SOURCE})
