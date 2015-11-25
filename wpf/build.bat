cl /clr /AI "c:\Program Files (x86)\Reference Assemblies\Microsoft\Framework\.NETFramework\v4.0" wpf.cpp /link /dll /out:wpf.dll ole32.lib

cl /Tc program.c /link /out:program.exe wpf.lib
