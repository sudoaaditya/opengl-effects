#version 460 core

in vec4 vPosition;
in vec4 vColor;

uniform mat4 u_modelMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_projMatrix;

out vec4 out_color;

void main(void) {

    gl_Position = u_projMatrix * u_viewMatrix * u_modelMatrix * vPosition;

    out_color = vColor;
}
