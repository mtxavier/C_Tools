#version 330

uniform vec2 InvScrDimx2;
in vec2 Coord;
in vec2 TextCoord;

noperspective out vec2 TexturePos;

void main() {
    gl_Position = vec4(
        (Coord.x*InvScrDimx2.x)-1.,
        1.-(Coord.y*InvScrDimx2.y),
        1.,1.
    );
    TexturePos = vec3(TextCoord.x,TextCoord.y);
}

