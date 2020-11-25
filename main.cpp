/*
 poslight.cpp
 Demonstrates a positional light with attenuation
 with per-vertex lighting (Gouraud shading) coded in the vertex shader.
 Displays a cube and a sphere and a small sphere to show the light position
 Includes controls to move the light source and rotate the view
 Iain Martin October 2018
*/

/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#pragma comment(lib, "glloadD.lib")
#else
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glload.lib")
#endif
#pragma comment(lib, "opengl32.lib")

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>
#include <stack>

   /* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// Include headers for our objects
#include "sphere.h"
#include "cube.h"

#include "tube.h"

/* Define buffer object indices */
GLuint elementbuffer;

GLuint program;		/* Identifier for the shader prgoram */
GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */
GLuint emitmode;
GLuint attenuationmode;

/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, model_scale, z, y, vx, vy, vz;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLfloat speed;				// movement increment
GLfloat motorAngle;				// movement increment

GLfloat light_x, light_y, light_z;

/* Uniforms*/
GLuint modelID, viewID, projectionID, normalMatrixID, viewPosID;
GLuint colourModeID, emitModeID, attenuationModeID;
GLuint colourOverrideID, reflectivenessID, numLightsID;

const int maxNumLights = 10;
GLuint lightPosID[maxNumLights];
int numLights;


GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
GLuint numspherevertices;

/* Global instances of our objects */
Tube tube;
Tube motorBell, motorStator, motorShaft;
Cube cube;
Sphere sphere;

using namespace std;
using namespace glm;

/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper* glw)
{
	/* Set the object transformation controls to their initial values */
	speed = 0.05f;
	x = 0.05f;
	y = 0;
	z = 0;
	vx = 0; vx = 0, vz = 4.f;
	light_x = 0; light_y = 1; light_z = 0;
	angle_x = angle_y = angle_z = 0;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	model_scale = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 0; 
	emitmode = 0;
	attenuationmode = 1; // Attenuation is on by default
	motorAngle = 0;
	numLights = 0;

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader(".\\poslight.vert", ".\\poslight.frag");
	}
	catch (exception& e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Define uniforms to send to vertex shader */
	modelID = glGetUniformLocation(program, "model");
	colourModeID = glGetUniformLocation(program, "colourMode");
	emitModeID = glGetUniformLocation(program, "emitMode");
	attenuationModeID = glGetUniformLocation(program, "attenuationMode");
	viewID = glGetUniformLocation(program, "view");
	projectionID = glGetUniformLocation(program, "projection");
	normalMatrixID = glGetUniformLocation(program, "normalMatrix");
	for (int i = 0; i < maxNumLights; i++)
	{
		std::string str = "lightPos[" + std::to_string(i) + "]";
		lightPosID[i] = glGetUniformLocation(program, str.c_str());
	}
	numLightsID = glGetUniformLocation(program, "numLights");
	viewPosID = glGetUniformLocation(program, "viewPos");
	colourOverrideID = glGetUniformLocation(program, "colourOverride");
	reflectivenessID = glGetUniformLocation(program, "reflectiveness");

	/* create our sphere and cube objects */

	sphere.makeSphere(20, 20);
	tube.makeTube(40, 0.1);
	motorBell.makeTube(40, 0.1);
	motorStator.makeTube(40, 0.85);
	motorShaft.makeTube(40, 0.7);
	cube.makeCube();
}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	glClearColor(0.2f, 0.5f, 1.0f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);

	/* Make the compiled shader program current */
	glUseProgram(program);

	// Define our model transformation in a stack and 
	// push the identity matrix onto the stack
	stack<mat4> model;
	model.push(mat4(1.0f));

	// Define the normal matrix
	mat3 normalmatrix;

	// Projection matrix : 60° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(60.f), aspect_ratio, 0.1f, 100.f);

	// Camera matrix
	mat4 view = lookAt(
		vec3(0, 0, 4), // Camera is at (0,0,4), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Apply rotations to the view position. This wil get appleid to the whole scene
	view = rotate(view, -radians(vx), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	view = rotate(view, -radians(vy), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	view = rotate(view, -radians(vz), vec3(0, 0, 1));

	// Define the light position and transform by the view matrix
	vec4 lightpos = view * vec4(light_x, light_y, light_z, 1.0);

	numLights = 0;

	// Send our projection and view uniforms to the currently bound shader
	// I do that here because they are the same for all objects
	glUniform1ui(colourModeID, colourmode);
	glUniform1ui(attenuationModeID, attenuationmode);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniform4fv(lightPosID[0], 1, value_ptr(lightpos));
	glUniform1ui(numLightsID, ++numLights);

	/* Draw a small sphere in the lightsource position to visually represent the light source */
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(light_x, light_y, light_z));
		model.top() = scale(model.top(), vec3(0.05f, 0.05f, 0.05f)); // make a small sphere
																	 // Recalculate the normal matrix and send the model and normal matrices to the vertex shader																							// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																						// Recalculate the normal matrix and send to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our lightposition sphere  with emit mode on*/
		emitmode = 1;
		glUniform1ui(emitModeID, emitmode);
		sphere.drawSphere(drawmode);
		emitmode = 0;
		glUniform1ui(emitModeID, emitmode);
	}
	model.pop();

	// Define the global model transformations (rotate and scale). Note, we're not modifying thel ight source position
	model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));//scale equally in all axis
	model.top() = rotate(model.top(), -radians(angle_x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model.top() = rotate(model.top(), -radians(angle_y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model.top() = rotate(model.top(), -radians(angle_z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	// This block of code draws the drone
	model.push(model.top());
	{

		model.top() = translate(model.top(), vec3(x, y, z));
		
		vec3 framePlateScale = vec3(1.f, 0.015f, 0.3f);
		vec3 frameArmScale = vec3(0.8f, 0.03f, 0.15f);
		vec3 standoffScale = vec3(0.025f, 0.17f, 0.025f);

		vec3 motorBellScale = vec3(0.15f, 0.085f, 0.15f);
		vec3 motorStatorScale = vec3(0.125f, 0.08f, 0.125f);
		vec3 motorShaftScale = vec3(0.025f, 0.085f, 0.025f);
		vec3 motorStrutsScale = vec3(0.011f, 0.011f, 0.14f);

		vec4 frameColour = vec4(0.20f, 0.20f, 0.20f, 1.f);
		vec4 motorColour = vec4(0.60f, 0.60f, 0.60f, 1.f);
		vec4 motorStatorColour = vec4(0.88f, 0.44f, 0.f, 1.f);
		vec4 standoffColour = vec4(1.f, 0.f, 0.f, 1.f);

		GLfloat frameReflect = 0.f;
		GLfloat motorReflect = 10.f;
		GLfloat motorStatorReflect = 2.f;
		GLfloat standoffReflect = 4.f;

		model.push(model.top());
		{

			model.top() = translate(model.top(), vec3(0.f, -0.085f, 0.f));
			model.top() = scale(model.top(), framePlateScale);

			// set the reflectiveness uniform
			glUniform1f(reflectivenessID, frameReflect);
			// set the colour uniform
			glUniform4fv(colourOverrideID, 1, &frameColour[0]);
			// Send the model uniform and normal matrix to the currently bound shader,
			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			// Recalculate the normal matrix and send to the vertex shader
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

			/* Draw our cube*/
			cube.drawCube(drawmode);
		}
		model.pop();

		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(0.f, 0.085f, 0.f));
			model.top() = scale(model.top(), framePlateScale);

			// set the reflectiveness uniform
			glUniform1f(reflectivenessID, frameReflect);
			// set the colour uniform
			glUniform4fv(colourOverrideID, 1, &frameColour[0]);
			// Send the model uniform and normal matrix to the currently bound shader,
			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			// Recalculate the normal matrix and send to the vertex shader
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

			/* Draw our cube*/
			cube.drawCube(drawmode);
		}
		model.pop();

		// arms
		
		for (int i = 0; i < 4; i++)
		{
			model.push(model.top());
			{
				model.top() = rotate(model.top(), -radians((90 * i) + 45.f), glm::vec3(0, 1, 0));
				model.top() = translate(model.top(), vec3(0.45f, -0.1f, 0.f));
				model.top() = scale(model.top(), frameArmScale);

				// set the reflectiveness uniform
				glUniform1f(reflectivenessID, frameReflect);
				// set the colour uniform
				glUniform4fv(colourOverrideID, 1, &frameColour[0]);	
				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(view * model.top())));
				glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

				cube.drawCube(drawmode);
			}
			model.pop();
		}


		//motors 
		for (int i = 0; i < 4; i++)
		{
			model.push(model.top());
			{

				model.push(model.top());
				{
					model.top() = rotate(model.top(), -radians((90 * i) + 45.f), glm::vec3(0, 1, 0));
					model.top() = translate(model.top(), vec3(0.77f, -0.02f, 0.f));


					// moving bits
					model.push(model.top());
					{

						if (i % 2 == 0)
						{
							model.top() = rotate(model.top(), -radians(motorAngle++), glm::vec3(0, 1, 0));
						}
						else
						{
							model.top() = rotate(model.top(), radians(motorAngle++), glm::vec3(0, 1, 0));
						}
					
						// mtor struts
						model.push(model.top());
						{
							model.top() = translate(model.top(), vec3(0.f, 0.042f, 0.f));
							for (int j = 0; j < 3; j++)
							{
								model.push(model.top());
								{
									model.top() = rotate(model.top(), -radians(120.f * j), glm::vec3(0, 1, 0));
									model.push(model.top());
									{
										model.top() = translate(model.top(), vec3(0.015f, 0.f, 0.f));
										model.top() = scale(model.top(), motorStrutsScale);

										// set the reflectiveness uniform
										glUniform1f(reflectivenessID, motorReflect);
										// set the colour uniform
										glUniform4fv(colourOverrideID, 1, &motorColour[0]);
										// Send the model uniform and normal matrix to the currently bound shader,
										glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
										// Recalculate the normal matrix and send to the vertex shader
										normalmatrix = transpose(inverse(mat3(view * model.top())));
										glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

										cube.drawCube(drawmode);
									}
									model.pop();

									model.push(model.top());
									{
										model.top() = translate(model.top(), vec3(-0.015f, 0.f, 0.f));
										model.top() = scale(model.top(), motorStrutsScale);

										// set the reflectiveness uniform
										glUniform1f(reflectivenessID, motorReflect);
										// set the colour uniform
										glUniform4fv(colourOverrideID, 1, &motorColour[0]);
										// Send the model uniform and normal matrix to the currently bound shader,
										glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
										// Recalculate the normal matrix and send to the vertex shader
										normalmatrix = transpose(inverse(mat3(view * model.top())));
										glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

										cube.drawCube(drawmode);
									}
									model.pop();
								}
								model.pop();
							}
						}
						model.pop();

						// motor shaft
						model.push(model.top());
						{
							model.top() = translate(model.top(), vec3(0.f, 0.06f, 0.f));
							model.top() = scale(model.top(), motorShaftScale);
							model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

							// set the reflectiveness uniform
							glUniform1f(reflectivenessID, motorReflect);
							// set the colour uniform
							glUniform4fv(colourOverrideID, 1, &motorColour[0]);
							// Send the model uniform and normal matrix to the currently bound shader,
							glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
							// Recalculate the normal matrix and send to the vertex shader
							normalmatrix = transpose(inverse(mat3(view * model.top())));
							glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

							motorShaft.drawTube(drawmode);
						}
						model.pop();

						// motor bell
						model.push(model.top());
						{
							model.top() = scale(model.top(), motorBellScale);
							model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

							// set the reflectiveness uniform
							glUniform1f(reflectivenessID, motorReflect);
							// set the colour uniform
							glUniform4fv(colourOverrideID, 1, &motorColour[0]);
							// Send the model uniform and normal matrix to the currently bound shader,
							glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
							// Recalculate the normal matrix and send to the vertex shader
							normalmatrix = transpose(inverse(mat3(view * model.top())));
							glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

							motorBell.drawTube(drawmode);
						}
						model.pop();
					}
					model.pop();
					
					// motor base 
					model.push(model.top());
					{
						model.top() = translate(model.top(), vec3(0.f, -0.06f, 0.f));
						model.top() = scale(model.top(), vec3(0.12f, 0.01f, 0.04f));

						// set the reflectiveness uniform
						glUniform1f(reflectivenessID, motorReflect);
						// set the colour uniform
						glUniform4fv(colourOverrideID, 1, &motorColour[0]);
						// Send the model uniform and normal matrix to the currently bound shader,
						glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
						// Recalculate the normal matrix and send to the vertex shader
						normalmatrix = transpose(inverse(mat3(view * model.top())));
						glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

						cube.drawCube(drawmode);						
					}
					model.pop();

					model.push(model.top());
					{
						model.top() = translate(model.top(), vec3(0.f, -0.06f, 0.f));
						model.top() = scale(model.top(), vec3(0.04f, 0.01f, 0.12f));

						// set the reflectiveness uniform
						glUniform1f(reflectivenessID, motorReflect);
						// set the colour uniform
						glUniform4fv(colourOverrideID, 1, &motorColour[0]);
						// Send the model uniform and normal matrix to the currently bound shader,
						glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
						// Recalculate the normal matrix and send to the vertex shader
						normalmatrix = transpose(inverse(mat3(view * model.top())));
						glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

						/* Draw our cube*/
						cube.drawCube(drawmode);
					}
					model.pop();

					// motor stator
					model.push(model.top());
					{
						model.top() = translate(model.top(), vec3(0.f, -0.015f, 0.f));
						model.top() = scale(model.top(), motorStatorScale);
						model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

						// set the reflectiveness uniform
						glUniform1f(reflectivenessID, motorStatorReflect);
						// set the colour uniform
						glUniform4fv(colourOverrideID, 1, &motorStatorColour[0]);
						// Send the model uniform and normal matrix to the currently bound shader,
						glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
						// Recalculate the normal matrix and send to the vertex shader
						normalmatrix = transpose(inverse(mat3(view * model.top())));
						glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

						/* Draw our cube*/
						motorStator.drawTube(drawmode);
					}
					model.pop();

				}
				model.pop();
			}
			model.pop();
		}


		model.push(model.top());
		{
		}
		model.pop();

		// standoffs
		{
			
			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(0.45f, 0.00f, 0.12f));
				model.top() = scale(model.top(), standoffScale);
				model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

				// set the reflectiveness uniform
				glUniform1f(reflectivenessID, standoffReflect);
				// set the colour uniform
				glUniform4fv(colourOverrideID, 1, &standoffColour[0]);
				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(view * model.top())));
				glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
			}
			model.pop();

			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(0.2f, 0.00f, 0.12f));
				model.top() = scale(model.top(), standoffScale);
				model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(view * model.top())));
				glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
			}
			model.pop();

			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(-0.2f, 0.00f, 0.12f));
				model.top() = scale(model.top(), standoffScale);
				model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(view * model.top())));
				glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
			}
			model.pop();

			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(-0.45f, 0.00f, 0.12f));
				model.top() = scale(model.top(), standoffScale);
				model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(view * model.top())));
				glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
			}
			model.pop();

			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(0.45f, 0.00f, -0.12f));
				model.top() = scale(model.top(), standoffScale);
				model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(view * model.top())));
				glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
			}
			model.pop();

			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(0.2f, 0.00f, -0.12f));
				model.top() = scale(model.top(), standoffScale);
				model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(view * model.top())));
				glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
			}
			model.pop();

			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(-0.2f, 0.00f, -0.12f));
				model.top() = scale(model.top(), standoffScale);
				model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(view * model.top())));
				glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
			}
			model.pop();

			model.push(model.top());
			{
				model.top() = translate(model.top(), vec3(-0.45f, 0.00f, -0.12f));
				model.top() = scale(model.top(), standoffScale);
				model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

				// Send the model uniform and normal matrix to the currently bound shader,
				glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

				// Recalculate the normal matrix and send to the vertex shader
				normalmatrix = transpose(inverse(mat3(view * model.top())));
				glUniformMatrix3fv(normalMatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

				/* Draw our cube*/
				tube.drawTube(drawmode);
			}
			model.pop();

		}
	}
	model.pop();


	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify our animation variables */
	angle_x += angle_inc_x;
	angle_y += angle_inc_y;
	angle_z += angle_inc_z;
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f * 4.f) / ((float)h / 480.f * 3.f);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	/* Enable this call if you want to disable key responses to a held down key*/
	//if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == 'Q') angle_inc_x -= speed;
	if (key == 'W') angle_inc_x += speed;
	if (key == 'E') angle_inc_y -= speed;
	if (key == 'R') angle_inc_y += speed;
	if (key == 'T') angle_inc_z -= speed;
	if (key == 'Y') angle_inc_z += speed;
	if (key == 'A') model_scale -= speed / 0.5f;
	if (key == 'S') model_scale += speed / 0.5f;
	if (key == 'Z') x -= speed;
	if (key == 'X') x += speed;
	if (key == 'C') y -= speed;
	if (key == 'V') y += speed;
	if (key == 'B') z -= speed;
	if (key == 'N') z += speed;
	if (key == '1') light_x -= speed;
	if (key == '2') light_x += speed;
	if (key == '3') light_y -= speed;
	if (key == '4') light_y += speed;
	if (key == '5') light_z -= speed;
	if (key == '6') light_z += speed;
	if (key == '7') vx -= 1.f;
	if (key == '8') vx += 1.f;
	if (key == '9') vy -= 1.f;
	if (key == '0') vy += 1.f;
	if (key == 'O') vz -= 1.f;
	if (key == 'P') vz += 1.f;

	if (key == 'M' && action != GLFW_PRESS)
	{
		colourmode = !colourmode;
		cout << "colourmode=" << colourmode << endl;
	}

	/* Turn attenuation on and off */
	if (key == '.' && action != GLFW_PRESS)
	{
		attenuationmode = !attenuationmode;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == ',' && action != GLFW_PRESS)
	{
		drawmode++;
		if (drawmode > 2) drawmode = 0;
	}
}


/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper* glw = new GLWrapper(1024, 768, "Position light example");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	/* Output the OpenGL vendor and version */
	glw->DisplayVersion();

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}

