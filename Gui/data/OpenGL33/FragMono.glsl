#version 330

uniform vec4 Foreground;
out vec4 gl_FragColor;

void main() {
    gl_FragColor = Foreground;
}

