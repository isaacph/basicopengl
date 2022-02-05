#version 330

uniform mat4 matrix;
uniform vec2 min;
uniform vec2 max;

in vec2 position;
in vec2 texture;

out vec2 texCoord;

void main() {
    gl_Position = matrix * vec4(position, 0, 1);
    texCoord = min + texture * (max - min);
}
