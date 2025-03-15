#version 460 core

in vec3 aPosition;
in vec2 aTexCoords;

uniform mat4 mvpMatrix;

out vec2 out_texCoords;

void main(void) {

    gl_Position = mvpMatrix * vec4(aPosition, 1.0);
    out_texCoords = aPosition.xy;
}
