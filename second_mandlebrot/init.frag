#version 400

uniform dvec2 center;
uniform double scale;
uniform float aspect;

in vec2 UV;
out vec2 coords;

void main() {
    vec2 c;
    c = vec2(aspect * (UV.x) * scale + center.x,
                       (UV.y) * scale + center.y);

    coords = vec2(c.x, c.y);
}
