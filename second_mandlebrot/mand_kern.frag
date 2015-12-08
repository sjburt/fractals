#version 430

layout (binding = 0, offset = 0) uniform atomic_uint count_iters;

in vec2 UV;

uniform sampler2D in_c;
uniform sampler2D in_z;
uniform int iters_to_do;
out vec4 diffuseColor;



void main() {
    float dist;

    vec2 c = texture(in_c, UV).xy;
    vec2 z = texture(in_z, UV).xy;
    float i = texture(in_z, UV).z;
    for (int j = 0; j < iters_to_do; j++) {
        dist = dot(z,z);
        if (dist < 4.0) {
            z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y ) + c;
            i++;
        }
        else
            break;
        // atomicCounterIncrement(count_iters);
    }
    diffuseColor = vec4(z.x, z.y, i, dist);
}
