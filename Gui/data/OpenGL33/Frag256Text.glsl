#version 330 

uniform usampler2DRect CurrentPTexture;
uniform sampler2DRect CurrentPalette;
uniform int CurrentPaletteBase;
uniform uint ColorKey;

noperspective in vec2 TexturePos; 

out vec4 gl_FragColor;

void main() {
    int x,idx;
    uint dts;
    x = int(TexturePos.x);
    dts = texelFetch(CurrentPTexture,ivec2(int(TexturePos.x),int(TexturePos.y))).r;
    if (dts==ColorKey) {
        discard;
    } else {
        idx = CurrentPaletteBase+int(dts);
        gl_FragColor = texelFetch(CurrentPalette,ivec2(idx&0xff,idx>>8));
    }
}

