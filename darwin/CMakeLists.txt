
set(LIBUI_COCOA_SOURCES
    ../ui_darwin.h
    alloc.m
    area.m
    areaevents.m
    autolayout.m
    box.m
    button.m
    checkbox.m
    colorbutton.m
    combobox.m
    control.m
    datetimepicker.m
    debug.m
    draw.m
    drawtext.m
    editablecombo.m
    entry.m
    fontbutton.m
    group.m
    label.m
    main.m
    map.m
    menu.m
    multilineentry.m
    progressbar.m
    radiobuttons.m
    scrollview.m
    separator.m
    slider.m
    spinbox.m
    stddialogs.m
    tab.m
    text.m
    util.m
    window.m
)

add_library(libui ${LIBUI_COMMON_SOURCES} ${LIBUI_COCOA_SOURCES})

