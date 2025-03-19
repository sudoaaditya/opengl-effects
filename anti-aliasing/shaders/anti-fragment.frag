#version 460 core

precision mediump float;
precision lowp int;

in vec4 out_color;
out vec4 FragColor;

void main(void) {
    FragColor = out_color;
}
