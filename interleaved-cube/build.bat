del *.exe
del *.obj
del *.txt

cls

cl.exe /c /EHsc /I C:\glew\include Cube.cpp clockUtils\Clock.cpp

rc.exe TexResource.rc

link.exe Cube.obj CLock.obj TexResource.res /LIBPATH:C:\glew\lib\Release\x64 opengl32.lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS 

Cube.exe