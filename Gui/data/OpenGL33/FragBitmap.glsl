#version 330

uniform usampler2DRect CurrentPTexture;
uniform vec4 Foreground;
uniform vec4 Background;

noperspective in vec2 TexturePos;

out vec4 gl_FragColor;

void main() {
    int x,rx;
    uint dts;
    x = int(TexturePos.x); rx=x&7; x=x>>3;
    dts = texelFetch(CurrentPTexture,ivec2(x,int(TexturePos.y))).r;
    dts = (dts>>rx)&1u;
    gl_FragColor = (bool(dts))?Foreground:Background;
}

