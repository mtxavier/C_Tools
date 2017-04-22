#version 330

uniform vec2 InvScrDimx2;
in vec2 Coord;

void main() {
    gl_Position = vec4(
        (Coord.x*InvScrDimx2.x)-1.,
        1.-(Coord.y*InvScrDimx2.y),
        0.,1.
    );
}

