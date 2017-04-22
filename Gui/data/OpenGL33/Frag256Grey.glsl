#version 330

uniform usampler2DRect CurrentPTexture;
uniform vec4 Foreground;
uniform vec4 Background;
uniform uint ColorKey;

noperspective in vec2 TexturePos;

out vec4 gl_FragColor;

void main() {
    uint dts;
    dts = texelFetch(CurrentPTexture,ivec2(int(TexturePos.x),int(TexturePos.y))).r;
    if (dts==ColorKey) {
        discard;
    } else {
        gl_FragColor = mix(Background,Foreground,float(dts)/255.);
    }
}

