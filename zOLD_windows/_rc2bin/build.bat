@rem 2 may 2018
@echo off

cl /nologo /TP /GR /EHsc /MDd /Ob0 /Od /RTC1 /W4 /wd4100 /bigobj /RTC1 /RTCs /RTCu /FS -c main.cpp
if errorlevel 1 goto out
rc -foresources.res resources.rc
if errorlevel 1 goto out
link /nologo main.obj resources.res /out:main.exe /LARGEADDRESSAWARE /NOLOGO /INCREMENTAL:NO /MANIFEST:NO /debug user32.lib kernel32.lib gdi32.lib comctl32.lib uxtheme.lib msimg32.lib comdlg32.lib d2d1.lib dwrite.lib ole32.lib oleaut32.lib oleacc.lib uuid.lib  

:out
