#version 400

uniform dvec2 center;
uniform double scale;
uniform float aspect;

in vec2 UV;
out vec2 coords;

void main() {
    vec2 cc = vec2(aspect * (2*UV.x-1) * scale + center.x,
                            (2*UV.y-1) * scale + center.y);

    coords = vec2(cc.x, cc.y);
}
