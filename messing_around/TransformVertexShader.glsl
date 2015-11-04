#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec4 v_m;
layout(location = 1) in vec2 vertexUV;

// Output data ; will be interpolated for each fragment.
out vec2 UV;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  v_m;
	
	//gl_Position = vec4(v_m.x, v_m.y, 0, 1);

	// UV of the vertex. No special space for this one.
	UV = vertexUV;
}

