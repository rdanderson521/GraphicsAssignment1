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

out vec3 vertexPos;
out vec3 vertexNormal;
out vec3 lightDir;
out vec4 lightColour;


// These are the uniforms that are defined in the application
uniform mat4 model, view, projection;
uniform mat3 normalmatrix;
uniform uint colourmode, emitmode;
uniform vec4 lightpos;

uniform uint attenuationmode;

// Global constants (for this vertex shader)
vec3 specular_albedo = vec3(1.0, 0.8, 0.6);
vec3 global_ambient = vec3(0.05, 0.05, 0.05);
int  shininess = 8;

void main()
{
	lightColour = colour;
	vertexPos = position;
	vertexNormal = normal; 

	vec4 position_h = vec4(position, 1.0);	// Convert the (x,y,z) position to homogeneous coords (x,y,z,w)

	gl_Position = (projection * view * model) * position_h;
}


