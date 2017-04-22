#version 330 

uniform usampler2DRect CurrentPTexture;
uniform sampler2DRect CurrentPalette;
uniform int CurrentPaletteBase;
uniform uint ColorKey;

noperspective in vec2 TexturePos;

out vec4 gl_FragColor;

void main() {
    int x,sx,idx;
    uint dts;
    x = int(TexturePos.x); sx=(x&1)<<2; x=x>>1;
    dts = texelFetch(CurrentPTexture,ivec2(x,int(TexturePos.y))).r;
    dts = (dts>>sx)&15u;
    if (dts==ColorKey) {
        discard;
    } else {
        idx = CurrentPaletteBase+int(dts);
        gl_FragColor = texelFetch(CurrentPalette,ivec2(idx&0xff,idx>>8));
    }
}

