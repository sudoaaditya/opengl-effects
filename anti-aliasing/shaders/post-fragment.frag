#version 460 core

in vec2 out_texCoords;
out vec4 FragColor;

uniform sampler2D screenTex;

void main (void) {
    // vec3 color = texture(screenTex, out_texCoords).rgb;
    // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    vec4 color = texture(screenTex, out_texCoords);
    // FragColor = mix(vec4(1.0, 0.0, 0.0, 1.0), color, 0.8);
    FragColor = color;
}
