#ifndef TUBE_H
#define TUBE_H

#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>

class Tube
{
public:
	Tube();
	~Tube();

	void makeTube(GLuint numSegments, GLfloat thickness);
	void drawTube(int drawmode);

	// Define vertex buffer object names (e.g as globals)
	GLuint tubeBufferObject;
	GLuint tubeNormals;
	GLuint tubeColours;
	GLuint elementbuffer;

	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colours;

	int numTubeVertices;
	int numSegments;
	float thickness;

private:
	void makeUnitTube(GLfloat* pVertices);
};


#endif