#version 120

uniform vec2 InvScrDimx2;
uniform vec2 InvMapDim;
attribute vec2 Coord;
attribute vec2 TextCoord;

varying vec3 TexturePos;

void main() {
    gl_Position = vec4(
        (Coord.x*InvScrDimx2.x)-1.,
        1.-(Coord.y*InvScrDimx2.y),
        0.,1.
    );
    TexturePos = vec3(TextCoord.x*InvMapDim.x,TextCoord.y*InvMapDim.y,TextCoord.x);
}

