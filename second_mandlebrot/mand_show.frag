#version 330

uniform int iter;

in vec2 UV;

uniform sampler1D colors;
uniform sampler2D location;

out vec4 diffuseColor;

void main() {

    float dist = texture(location, UV).w;
    float i = texture(location, UV).z;
    diffuseColor = (dist < 4.0) ? vec4(0.0) : texture(colors, (float(i)/200.0));
}
