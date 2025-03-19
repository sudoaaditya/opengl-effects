#version 460 core

out vec4 FragColor;

uniform sampler2DMS screenTex;
uniform int toggleAliasing;

void main(void) {

    ivec2 texPos = ivec2(gl_FragCoord.x, gl_FragCoord.y);

    vec4 color = texelFetch(screenTex, texPos, 0);

    if(toggleAliasing == 1) {
        vec4 sample1 = texelFetch(screenTex, texPos, 1);
        vec4 sample2 = texelFetch(screenTex, texPos, 2);
        vec4 sample3 = texelFetch(screenTex, texPos, 3);

        color = (color + sample1 + sample2 + sample3) / 4.0;
    }

    FragColor = color;
}
