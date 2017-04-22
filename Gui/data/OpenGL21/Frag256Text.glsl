#version 120

uniform sampler2D CurrentPTexture;
uniform sampler2D CurrentPalette;
uniform int CurrentPaletteBase;
uniform int ColorKey;

varying vec3 TexturePos; 

void main() {
    vec4 ColNum;
    int idx;
    idx = CurrentPaletteBase+int(texture2D(CurrentPTexture,vec2(TexturePos.x,TexturePos.y)).r*255.);
    if (ColorKey!=idx) {
        gl_FragColor = texture2D(CurrentPalette,vec2(mod(idx,256.)/255.,floor(idx/256.)/31.));
    } else
        discard;
}

