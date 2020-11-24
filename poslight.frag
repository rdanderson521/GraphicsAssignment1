// Minimal fragment shader
// Iain Martin 2018

#version 420 core

//in vec4 fcolour;

in vec3 vertexPos;
in vec3 vertexNormal;
in vec3 lightDir;
in vec4 lightColour;

out vec4 outputColor;

uniform mat4 model, view, projection;
uniform mat3 normalmatrix;
uniform uint colourmode, emitmode;
uniform vec4 lightpos;

uniform uint attenuationmode;

vec3 specular_albedo = vec3(1.0, 0.8, 0.6);
vec3 global_ambient = vec3(0.05, 0.05, 0.05);
int  shininess = 20;

void main()
{
	vec3 emissive = vec3(0);
	vec4 position_h = vec4(vertexPos, 1.0);
	vec4 diffuse_albedo;					// This is the vertex colour, used to handle the colourmode change
	vec3 light_pos3 = lightpos.xyz;			

	// Switch the vertex colour based on the colourmode
	if (colourmode == 1)
		diffuse_albedo = lightColour;
	else
		diffuse_albedo = vec4(1.0, 0, 0, 1.0);
	

	vec3 ambient = diffuse_albedo.xyz *0.2;

	// Define our vectors to calculate diffuse and specular lighting
	mat4 mv_matrix = view * model;		// Calculate the model-view transformation
	vec4 P = mv_matrix * position_h;	// Modify the vertex position (x, y, z, w) by the model-view transformation
	vec3 N = normalize(normalmatrix * vertexNormal);		// Modify the normals by the normal-matrix (i.e. to model-view (or eye) coordinates )
	vec3 L = light_pos3 - P.xyz;		// Calculate the vector from the light position to the vertex in eye space
	float distanceToLight = length(L);	// For attenuation
	L = normalize(L);					// Normalise our light vector

	// Calculate the diffuse component
	vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo.xyz;

	// Calculate the specular component using Phong specular reflection
	vec3 V = normalize(-P.xyz);	
	vec3 R = reflect(-L, N);
	vec3 specular = pow(max(dot(R, V), 0.0), shininess) * specular_albedo;

	// Calculate the attenuation factor;
	float attenuation;
	if (attenuationmode != 1)
	{
		attenuation = 1.0;
	}
	else
	{
		// Define attenuation constants. These could be uniforms for greater flexibility
		float attenuation_k1 = 0.5;
		float attenuation_k2 = 0.5;
		float attenuation_k3 = 0.5;
		attenuation = 1.0 / (attenuation_k1 + attenuation_k2*distanceToLight + 
								   attenuation_k3 * pow(distanceToLight, 2));
	}

	if (emitmode == 1) emissive = vec3(1.0, 1.0, 0.8);

	outputColor =  vec4(attenuation*(specular + ambient + diffuse) + emissive +  global_ambient,1.0);
}