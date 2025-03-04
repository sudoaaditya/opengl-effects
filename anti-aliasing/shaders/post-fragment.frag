#version 460 core

precision mediump float;
precision lowp int;

in vec2 out_texCoords;
out vec4 FragColor;

uniform sampler2D screenTex;

void main (void) {
    // vec3 color = texture(screenTex, out_texCoords).rgb;
    // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    FragColor = texture(screenTex, out_texCoords);
}
