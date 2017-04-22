#version 120 

uniform sampler2D CurrentPTexture;
uniform sampler2D CurrentPalette;
uniform vec2 InvMapDim;
uniform int CurrentPaletteBase;
uniform int ColorKey;

varying vec3 TexturePos; 

const float divis[4] = float[4](4.,16.,64.,256.);

void main() {
    float x,sx,dts;
    float idx;
    x = TexturePos.z; sx=divis[int(mod(x,4.))]; x=InvMapDim.x*floor(x/4.);
    dts = texture2D(CurrentPTexture,vec2(x,TexturePos.y)).r;
    idx = CurrentPaletteBase+mod(floor(dts*sx),4.);
    if (idx==ColorKey) {
        discard;
    } else {
        gl_FragColor = texture2D(CurrentPalette,vec2(mod(idx,256.)/255.,floor(idx/256.)/31.));
    }
}

