#version 330

uniform mat4x4 Colors;

noperspective in vec2 TexturePos;

out vec4 gl_FragColor;

void main() {
    float u,v;
    vec4 col0,col1;
    u = fract(TexturePos.x);
    v = fract(TexturePos.y);
    col0 = mix(Colors[1],Colors[0],u);
    col1 = mix(Colors[2],Colors[3],u);
    gl_FragColor = mix(col1,col0,v);
}

