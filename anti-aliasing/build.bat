del *.exe
del *.obj
del *.txt

cls

cl.exe /c /EHsc /I C:\glew\include AACube.cpp clockUtils\Clock.cpp shaderUtils\LoadShaders.cpp

rc.exe TexResource.rc

link.exe AACube.obj Clock.obj LoadShaders.obj TexResource.res /LIBPATH:C:\glew\lib\Release\x64 opengl32.lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS 

AACube.exe