#version 400

uniform dvec2 center;
uniform double scale;
uniform int iter;
uniform float aspect;

uniform sampler1D colors;

in vec2 UV;
out vec4 diffuseColor;

void main() {
    dvec2 c, z;
	c = dvec2(aspect * (UV.x - 0.5) * scale + center.x,
                       (UV.y - 0.5) * scale + center.y);
    z = c;
	double dist;
	int i = 0;

	for(i= 0 ; i < iter; i++) {
        dist = dot(z,z);
        if (dist > 4.0) {
            break;
        }
        dmat2 A = dmat2(z.x, z.y, -z.y, z.x);
        z = (A * z) + c;
    }
    diffuseColor = (i==iter) ? vec4(0) : texture(colors, (float(i)/200.0));
}
