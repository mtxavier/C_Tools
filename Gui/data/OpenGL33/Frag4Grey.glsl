#version 330

uniform usampler2DRect CurrentPTexture;
uniform vec4 Foreground;
uniform vec4 Background;
uniform uint ColorKey;

noperspective in vec2 TexturePos;

out vec4 gl_FragColor;

void main() {
    uint dts;
    int x,rx;
    x = int(TexturePos.x); rx = (x&3)<<1; x=x>>2;
    dts = ((texelFetch(CurrentPTexture,ivec2(x,int(TexturePos.y))).r)>>rx)&3u;
    if (dts==ColorKey) {
        discard;
    } else {
        gl_FragColor = mix(Background,Foreground,float(dts)/3.);
    }
}

