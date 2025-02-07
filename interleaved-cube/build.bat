del *.exe
del *.obj
del *.txt

cls

cl.exe /c /EHsc /I C:\glew-2.1.0\include Cube.cpp

rc.exe TexResource.rc

link.exe Cube.obj TexResource.res /LIBPATH:C:\glew-2.1.0\lib\Release\x64 opengl32.lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS 

Cube.exe