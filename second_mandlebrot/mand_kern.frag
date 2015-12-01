#version 430

uniform int iter;
layout (binding = 0, offset = 0) uniform atomic_uint count_iters;

in vec2 UV;

uniform sampler1D colors;
uniform sampler2D location;

out vec4 diffuseColor;

void main() {
    float dist;
    int i;
    // vec2 z;

    vec2 c = texture(location, UV).xy;
    vec2 z = c;

    for(i = 0 ; i < iter; i++) {
        dist = dot(z,z);
        if (dist > 4.0) {
            break;
        }
        z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y ) + c;
        atomicCounterIncrement(count_iters);
    }
    diffuseColor = (i==iter) ? vec4(0) : texture(colors, (float(i)/100));
}
