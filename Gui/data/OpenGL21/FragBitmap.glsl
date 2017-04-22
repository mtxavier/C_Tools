#version 120

uniform sampler2D CurrentPTexture;
uniform vec4 Foreground;
uniform vec4 Background;
uniform vec2 InvMapDim;

varying vec3 TexturePos; 

const float divis[8] = float[8](2.,4.,8.,16.,32.,64.,128.,256.);
void main() {
    float dts,x;
    int rx;
    x = TexturePos.z; rx = int(mod(x,8.)); x=InvMapDim.x*floor(x/8.);
    dts = texture2D(CurrentPTexture,vec2(x,TexturePos.y)).r;
    dts = mod(floor(dts*divis[rx]),2.);
    gl_FragColor = (dts<.5)?Background:Foreground;
}

