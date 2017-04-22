#version 120

uniform sampler2D CurrentPTexture;
uniform vec4 Foreground;
uniform vec4 Background;
uniform vec2 InvMapDim;

varying vec3 TexturePos; 

void main() {
    float x,dts,rx;
    x = TexturePos.z; rx = mod(x,2.); x=InvMapDim.x*floor(x/2.);
    dts = floor(texture2D(CurrentPTexture,vec2(x,TexturePos.y)).r*255.);
    x = (rx>.5)? floor(dts/16.):mod(dts,16.);
    gl_FragColor = mix(Background,Foreground,x/15.);
}

