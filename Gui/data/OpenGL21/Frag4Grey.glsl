#version 120

uniform sampler2D CurrentPTexture;
uniform vec4 Foreground;
uniform vec4 Background;
uniform vec2 InvMapDim;

varying vec3 TexturePos; 

const float divis[4] = float[4](1.,1./4.,1./16.,1./64.);
void main() {
    float dts,x;
    int rx;
    x = TexturePos.z; rx = int(mod(x,4.)); x = InvMapDim.x*floor(x/4.);
    dts = floor(texture2D(CurrentPTexture,vec2(x,TexturePos.y)).r*255.);
    dts = mod(dts*divis[rx],4.);
    gl_FragColor = mix(Background,Foreground,dts/3.);
}

