#version 460 core

precision mediump float;
precision lowp int;

in vec4 out_color;

void main (void) {
    gl_FragColor = out_color;
}
