#version 120

uniform sampler2D CurrentPTexture;

varying vec3 TexturePos;

void main() {
    gl_FragColor = texture2D(CurrentPTexture,vec2(TexturePos.x,TexturePos.y));
}

