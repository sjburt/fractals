#version 430

uniform dvec2 center;
uniform double scale;
uniform int iter;
uniform float aspect;

uniform sampler1D colors;

layout (binding = 0, offset = 0) uniform atomic_uint count_iters;

in vec2 UV;
out vec4 diffuseColor;

void main() {
    dvec2 c, z;
    c = dvec2(aspect * (UV.x) * scale + center.x,
                       (UV.y) * scale + center.y);
    z = c;
    double dist;
    int i = 0;

    for(i= 0 ; i < iter; i++) {
        dist = dot(z,z);
        if (dist > 4.0) {
            break;
        }
        z = dvec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y ) + c;
        atomicCounterIncrement(count_iters);
    }
    diffuseColor = (i==iter) ? vec4(0) : texture(colors, .01 * (float(i)));
}
