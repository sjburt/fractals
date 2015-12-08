#version 330

uniform vec2 center;
uniform float scale;
uniform float aspect;

in vec2 UV;
out vec2 coords;

void main() {
    vec2 cc = vec2(aspect * (2*UV.x-1) * scale + center.x,
                            (2*UV.y-1) * scale + center.y);

    coords = vec2(cc.x, cc.y);
}
