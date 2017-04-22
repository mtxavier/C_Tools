#version 120

uniform sampler2D CurrentPTexture;
uniform sampler2D CurrentPalette;
uniform int CurrentPaletteBase;
uniform int ColorKey;
uniform vec2 InvMapDim;

varying vec3 TexturePos; 

void main() {
    float x,sx,dts,idx;
    x = TexturePos.z; idx=int(mod(x,2.)); x=InvMapDim.x*floor(x/2.);
    dts = floor(texture2D(CurrentPTexture,vec2(x,TexturePos.y)).r*255.);
    idx = CurrentPaletteBase+((idx==1)?floor(dts/16.):mod(dts,16.));
    if (ColorKey==idx) {
        discard;
    } else {
        gl_FragColor = texture2D(CurrentPalette,vec2(mod(idx,256.)/255.,floor(idx/256.)/31.));
    }
}

