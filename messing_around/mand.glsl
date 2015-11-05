#version 400 core

uniform dvec2 center;
uniform double scale;
uniform int iter;
uniform float aspect;

uniform sampler1D colors;

in vec2 UV;
out vec4 diffuseColor;

void main() {
    dvec2 c, z;
	c = dvec2(aspect * (UV.x - 0.5) * scale - center.x,
                      (UV.y - 0.5) * scale - center.y);
    z = c;
	double dist;
	int i = 0;

	for(i= 0 ; i < iter; i++) {
		dmat2 A = dmat2(z.x, z.y, -z.y, z.x);
		z = (A * z) + c;
		dist = dot(z,z);

		if (dist > 4.0) {
		    break;
		}
    }

	diffuseColor = (i==iter) ? vec4(0) : texture(colors, (float(i)/40.0));
//	diffuseColor = vec4(sin(.1 * float(i)),
//	                    sin(.21 * float(i)),
//						sin(.43 * float(i)),
//						0.0);

}




//void main_old() {
//    vec2 c, z;
//	c = vec2(aspect * (UV.x - 0.5) * scale - center.x,
//                      (UV.y - 0.5) * scale - center.y);
//    z = c;
//	float dist;
//	int i = 0;

//	for(int k = 0 ; k < iter; k++) {
//		mat2 A = mat2(z.x, z.y, -z.y, z.x);
//		z = (A * z) + c;
//		dist = dot(z,z);

//		if (dist > 4.0) {
//			i = k;
//		    break;
//		}
//    }
//	gl_FragColor = vec4(cos(.1* float(i)),
//	                    sin(.21 * float(i)),
//						sin(.43 * float(i)),
//						0.0);
//}
