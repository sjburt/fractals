#version 430

uniform int iter;

in vec2 UV;

uniform sampler1D colors;
uniform sampler2D location;

out vec4 diffuseColor;

void main() {

    float i = texture(location, UV).z;
    diffuseColor = (i==iter) ? vec4(0) : texture(colors, (float(i)/100));
}
