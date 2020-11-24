#include "Tube.h"

#define PI 3.14159265358979f

#include <iostream>

Tube::Tube()
{
	attribute_v_coord = 0;
	attribute_v_colours = 1;
	attribute_v_normal = 2;
	numTubeVertices = 0;
}

Tube::~Tube()
{}

void Tube::makeTube(GLuint numSegments, GLfloat thickness)
{
	GLuint numvertices = 8 * (numSegments);

	// Store the number of sphere vertices in an attribute because we need it later when drawing it
	this->numTubeVertices = numvertices;
	this->numSegments = numSegments;
	if (thickness >= 1)
	{
		this->thickness = 1.f;
	}
	else if (thickness <= 0)
	{
		this->thickness = 0.f;
	}
	else
	{
		this->thickness = thickness;
	}

	// Create the temporary arrays to stro
	GLfloat* pVertices = new GLfloat[numvertices * 3];
	GLfloat* pNormals = new GLfloat[numvertices * 3];
	GLfloat* pColours = new GLfloat[numvertices * 4];
	this->makeUnitTube(pVertices);

	for (int i = 0; i < numvertices; i++)
	{
		pColours[i * 4] = pVertices[i * 3];
		pColours[i * 4 + 1] = pVertices[i * 3 + 1];
		pColours[i * 4 + 2] = pVertices[i * 3 + 2];
		pColours[i * 4 + 3] = 1.f;
	}

	// sets normals for top and bottom disks
	for (int i = 0; i < numvertices / 2; i++)
	{
		pNormals[i * 3] = 0;
		pNormals[(i * 3) + 1] = 0;
		pNormals[(i * 3) + 2] = pVertices[(i * 3) + 2];
	}

	for (int i = numvertices / 2; i < numvertices; i++)
	{
		pNormals[i * 3] = pVertices[(i * 3)];
		pNormals[(i * 3) + 1] = pVertices[(i * 3) + 1];
		pNormals[(i * 3) + 2] = 0;
	}


	/* Generate the vertex buffer object */
	glGenBuffers(1, &this->tubeBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, this->tubeBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numvertices * 3, pVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Store the normals in a buffer object */
	glGenBuffers(1, &this->tubeNormals);
	glBindBuffer(GL_ARRAY_BUFFER, this->tubeNormals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numvertices * 3, pNormals, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Store the colours in a buffer object */
	glGenBuffers(1, &this->tubeColours);
	glBindBuffer(GL_ARRAY_BUFFER, this->tubeColours);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numvertices * 4, pColours, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint numindices = numvertices + 8;
	GLuint* pindices = new GLuint[numindices];

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < (numvertices/4); j++)
		{
			pindices[i * ((numvertices / 4)  + 2) + j] = i * (numvertices / 4) + j;
		}
		pindices[i * ((numvertices / 4) + 2) + (numvertices / 4)] = i * (numvertices / 4);
		pindices[i * ((numvertices / 4) + 2) + (numvertices / 4) + 1] = i * (numvertices / 4) + 1;
	}
	
	// Generate a buffer for the indices
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numindices * sizeof(GLuint), pindices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete pindices;
	delete pColours;
	delete pVertices;
}


void Tube::drawTube(int drawmode)
{
	GLuint i;

	/* Draw the vertices as GL_POINTS */
	glBindBuffer(GL_ARRAY_BUFFER, this->tubeBufferObject);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	/* Bind the sphere normals */
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, this->tubeNormals);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	/* Bind the sphere colours */
	glBindBuffer(GL_ARRAY_BUFFER, this->tubeColours);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glPointSize(3.f);

	// Enable this line to show model in wireframe
	if (drawmode == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (drawmode == 2)
	{
		glDrawArrays(GL_POINTS, 0, this->numTubeVertices);
	}
	else
	{
		/* Bind the indexed vertex buffer */
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		for (int i = 0; i < 4; i++)
		{
			glDrawElements(GL_TRIANGLE_STRIP, this->numSegments * 2 + 2, GL_UNSIGNED_INT, (GLvoid*)(i * ((this->numTubeVertices/4) + 2) * 4));
		}
	}
}


void Tube::makeUnitTube(GLfloat* pVertices)
{
	float segmentAngleIncrement = (2 * PI) / this->numSegments;

	int numVerticesPerSection = (this->numSegments * 2);

	// top disk
	for (int i = 0; i < this->numSegments; i++)
	{
		// to effectively set the index to 0 for the last vertices to finish the loop with repeating vertices
		//int segmentIdx = i % this->numSegments;

		// outside vertices
		pVertices[i * 6] = 0.5f * sin(segmentAngleIncrement * i);
		pVertices[(i * 6) + 1] = 0.5f * cos(segmentAngleIncrement * i);
		pVertices[(i * 6) + 2] = 0.5f;

		// inside vertices
		pVertices[(i * 6) + 3] = (0.5f - (this->thickness * 0.5f)) * sin(segmentAngleIncrement * i);
		pVertices[(i * 6) + 4] = (0.5f - (this->thickness * 0.5f)) * cos(segmentAngleIncrement * i);
		pVertices[(i * 6) + 5] = 0.5f;
	}

	for (int i = 0; i < this->numSegments * 6; i++)
	{
		// outside vertices
		if (i % 3 != 2)
		{
			// copies the x and y coordinates from the previous disk
			pVertices[(numVerticesPerSection * 3) + i] = pVertices[i];
		}
		else
		{
			// sets the z coordinate to the negative of the previous disk
			pVertices[(numVerticesPerSection * 3) + i] = -pVertices[i];
		}
	}
	
	// generates the outside surface
	for (int i = 0; i < this->numSegments; i++)
	{
		// copies outside vertices from the top disk
		pVertices[((numVerticesPerSection * 3) * 2) + (i * 6)] = pVertices[(i * 6)];
		pVertices[((numVerticesPerSection * 3) * 2) + (i * 6) + 1] = pVertices[(i * 6) + 1];
		pVertices[((numVerticesPerSection * 3) * 2) + (i * 6) + 2] = pVertices[(i * 6) + 2];

		// copies outside vertices from bottom disk
		pVertices[((numVerticesPerSection * 3) * 2) + (i * 6) + 3] = pVertices[(numVerticesPerSection * 3) + (i * 6) ];
		pVertices[((numVerticesPerSection * 3) * 2) + (i * 6) + 4] = pVertices[(numVerticesPerSection * 3) + (i * 6) + 1];
		pVertices[((numVerticesPerSection * 3) * 2) + (i * 6) + 5] = pVertices[(numVerticesPerSection * 3) + (i * 6) + 2];
	}

	// generates the inside surface
	for (int i = 0; i < this->numSegments; i++)
	{
		// copies outside vertices from the top disk
		pVertices[((numVerticesPerSection * 3) * 3) + (i * 6)] = pVertices[(i * 6) + 3];
		pVertices[((numVerticesPerSection * 3) * 3) + (i * 6) + 1] = pVertices[(i * 6) + 4];
		pVertices[((numVerticesPerSection * 3) * 3) + (i * 6) + 2] = pVertices[(i * 6) + 5];

		// copies outside vertices from bottom disk
		pVertices[((numVerticesPerSection * 3) * 3) + (i * 6) + 3] = pVertices[((numVerticesPerSection * 3))+(i * 6) + 3];
		pVertices[((numVerticesPerSection * 3) * 3) + (i * 6) + 4] = pVertices[((numVerticesPerSection * 3))+(i * 6) + 4];
		pVertices[((numVerticesPerSection * 3) * 3) + (i * 6) + 5] = pVertices[((numVerticesPerSection * 3))+(i * 6) + 5];
	}
}