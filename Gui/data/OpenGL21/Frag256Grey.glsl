#version 120

uniform sampler2D CurrentPTexture;
uniform vec4 Foreground;
uniform vec4 Background;

varying vec3 TexturePos; 

void main() {
    float blnd;
    blnd = texture2D(CurrentPTexture,vec2(TexturePos.x,TexturePos.y)).r;
    gl_FragColor = (blnd*Foreground)+((1.-blnd)*Background);
}

