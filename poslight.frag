// Minimal fragment shader
// Iain Martin 2018

#version 420 core

in VERTEX_OUT
{
	vec3 pos;
	vec3 normal;
	vec4 vertexColour;
	vec4 FragPosLightSpace;
} fIn;


out vec4 outputColor;

uniform vec3 viewPos;
uniform sampler2D shadowMap;

uniform mat4 model, view, projection;
uniform mat3 normalMatrix;

uniform uint emitMode;
uniform vec3 emitColour;
uniform vec4 lightPos[10];
uniform vec3 lightColour[10];
uniform uint lightMode[10];
uniform uint numLights;
uniform float reflectiveness; // value of 0.01 - 1

uniform uint attenuationMode;

vec3 specular_albedo = vec3(1.0, 0.8, 0.6);
vec3 global_ambient = vec3(0.05, 0.05, 0.05);

float shadowCalculation(vec4 lightSpace)
{

	vec3 projCoords = lightSpace.xyz / lightSpace.w;

	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowMap, projCoords.xy).r;

	float currentDepth = projCoords.z; 
	float bias = 0.005;
	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	if(projCoords.z > 1.0)
        shadow = 0.0;
    return shadow;
}


void main()
{
	vec3 emissive = vec3(0);
	if (emitMode == 1)
	{
		if (emitColour != vec3(0.f))
		{
			emissive = emitColour;
		}
		else
		{
			emissive = vec3(1.0, 1.0, 0.8);
		}
	
	}
	outputColor =  vec4((global_ambient * fIn.vertexColour.xyz) + emissive , 1.f);
	for (int i = 0; i < numLights; i++)
	{
		vec4 position_h = vec4(fIn.pos, 1.0);
		vec3 light_pos3 = lightPos[i].xyz;		
		
		vec3 currentLightColour = lightColour[i];
		if (lightColour[i] == vec3(0.f))
		{
			currentLightColour = vec3(1.0f);
		}

		vec3 ambient = fIn.vertexColour.xyz  * 0.1 * (0.8 + (0.2*currentLightColour));

		// Define our vectors to calculate diffuse and specular lighting
		mat4 mv_matrix = view * model;		// Calculate the model-view transformation
		vec4 P = view * position_h;	// Modify the vertex position (x, y, z, w) by the model-view transformation
		vec3 N = normalize(normalMatrix * fIn.normal);		// Modify the normals by the normal-matrix (i.e. to model-view (or eye) coordinates )
		vec3 L = light_pos3 - P.xyz;		// Calculate the vector from the light position to the vertex in eye space
		float distanceToLight = length(L);	// For attenuation
		L = normalize(L);					// Normalise our light vector

		// Calculate the diffuse component
		vec3 diffuse = max(dot(N, L), 0.0) * fIn.vertexColour.xyz * (0.2 + (0.8*currentLightColour));

		// Calculate the specular component using Phong specular reflection
		vec3 V = normalize(viewPos - P.xyz);	
		vec3 R = reflect(-L, N);
		vec3 specular = vec3(0.f);
		if (reflectiveness > 0.f)
		{
			specular = pow(max(dot(R, V), 0.0), 1/max(reflectiveness,0.0001) ) * specular_albedo * (0.8 + (0.2*currentLightColour));
		}

		// Calculate the attenuation factor;
		float attenuation;
		if (attenuationMode != 1)
		{
			attenuation = 1.0;
		}
		else
		{
			// Define attenuation constants. These could be uniforms for greater flexibility
			float attenuation_k1 = 0.5;
			float attenuation_k2 = 0.2;
			float attenuation_k3 = 0.8;
			attenuation = 1.0 / (attenuation_k1 + attenuation_k2*distanceToLight + 
									   attenuation_k3 * pow(distanceToLight, 2));
		}

		// calculate shadow value
		float shadow = 0.f;
		if (lightMode[i] == 1)
		{
			shadow = shadowCalculation(fIn.FragPosLightSpace);
		}

		outputColor +=  vec4(attenuation * (ambient + ((1.0 - shadow) * (specular + diffuse))), 1.0);
	}
}