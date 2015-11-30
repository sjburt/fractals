#version 430

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec4 v_m;
layout(location = 1) in vec2 vertexUV;

// Output data ; will be interpolated for each fragment.
out vec2 UV;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

void main(){
	gl_Position = v_m;
	// UV of the vertex. No special space for this one.
	UV = vertexUV;
}

