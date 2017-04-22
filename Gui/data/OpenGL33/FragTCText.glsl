#version 330

uniform sampler2DRect CurrentPTexture;

noperspective in vec2 TexturePos;

out vec4 gl_FragColor;

void main() {
    gl_FragColor = texelFetch(CurrentPTexture,ivec2(int(TexturePos.x),int(TexturePos.y)));
}

