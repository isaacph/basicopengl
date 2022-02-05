#version 330

uniform vec4 color;
uniform float radius;
uniform float thickness;

in vec2 texCoord;

out vec4 outColor;

void main() {
    vec2 diff = texCoord - vec2(0.5, 0.5);
    float dist = sqrt(diff.x * diff.x + diff.y * diff.y);
    if (abs(dist - radius) > thickness) {
        discard;
    }
    outColor = vec4(vec3(1), (thickness - abs(dist - radius)) / thickness) * color;
}
