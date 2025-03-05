#version 460 core

in vec2 aPosition;
in vec2 aTexCoords;


out vec2 out_texCoords;

void main (void) {

    gl_Position = vec4(aPosition.x, aPosition.y, 0.0, 1.0);
    out_texCoords = aTexCoords;
}


