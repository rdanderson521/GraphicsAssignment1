// Vertex shader demonstrating a positional light
// source with attenuation
// Iain Martin 2018

// Specify minimum OpenGL version
#version 420 core

// Define the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec3 normal;

// This is the output vertex colour sent to the rasterizer
//out vec4 fcolour;

out VERTEX_OUT
{
	vec3 pos;
	vec3 normal;
	vec4 vertexColour;
} vOut;



// These are the uniforms that are defined in the application
uniform mat4 model, view, projection;
uniform uint colourMode;
uniform vec4 colourOverride;

void main()
{
	if (colourMode == 1)
	{
		vOut.vertexColour = colourOverride;
	}
	else
	{
		vOut.vertexColour = colour;
	}
	vOut.pos = position;
	vOut.normal = normal; 

	gl_Position = (projection * view * model) * vec4(position, 1.0);
}


