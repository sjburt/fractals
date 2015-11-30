#version 430

uniform int iter;
layout (binding = 0, offset = 0) uniform atomic_uint count_iters;

layout (location = 0) in vec2 c;
// layout (location = 2) flat in dvec4 z;
// layout (location = 0) out vec4 outbuff;
uniform sampler1D colors;

out vec4 diffuseColor;

void main() {
    dvec2 z;
    double dist;
    int i;
    z = c;

    // for(i= 0 ; i < iter; i++) {
    //     dist = dot(z,z);
    //     if (dist > 4.0) {
    //         break;
    //     }
    //     z = dvec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y ) + c;
    //     atomicCounterIncrement(count_iters);
    // }
    // diffuseColor = (i==iter) ? vec4(0) : texture(colors, .01 * (float(i)));
    diffuseColor = vec4(float(c.x), float(c.y), 0.2, 0);
}















// void main() {
//     if (z.z == 0) {

//         dvec2 zz = dvec2(z.x*z.x - z.y*z.y,
//                          2.0*z.x*z.y )
//                     + dvec2(c.x, c.y);
//         atomicCounterIncrement(count_iters);

//         double dist = dot(zz,zz);

//         outbuff = vec4(float(zz.x), float(zz.y), (dist > 4.0) ? iter : 0, 0);
//     } else {
//         outbuff = vec4(float(z.x), float(z.y), 0, 0);
//     }
// }
