#include "stdafx.h"
#include "CubeObject.h"


CubeObject::CubeObject()
{
	this->x = 10;
	this->y = 0;
	this->z = 10;
	this->w = 10;
	this->h = 10;
	this->t = 10;
}

CubeObject::CubeObject(float px, float py, float pz, float pw, float ph, float pt)
{	
	this->x = px;
	this->y = py;
	this->z = pz;
	this->w = pw;
	this->h = ph;
	this->t = pt;
}

CubeObject::~CubeObject()
{
}

void CubeObject::Draw()
{
	// y is the height
	glBegin(GL_TRIANGLES);

	glVertex3f(x, y, z);
	glVertex3f(x + w, y + h, z);
	glVertex3f(x + w, y, z);

	glVertex3f(x, y, z);
	glVertex3f(x, y + h, z);
	glVertex3f(x + w, y + h, z);

	glVertex3f(x, y, z + t);
	glVertex3f(x + w, y, z + t);
	glVertex3f(x + w, y + h, z + t);

	glVertex3f(x, y, z + t);
	glVertex3f(x + w, y + h, z + t);
	glVertex3f(x, y + h, z + t);

	glVertex3f(x, y, z);
	glVertex3f(x, y, z + t);
	glVertex3f(x, y + h, z + t);

	glVertex3f(x, y, z);
	glVertex3f(x, y + h, z + t);
	glVertex3f(x, y + h, z);

	//
	glVertex3f(x + w, y, z);
	glVertex3f(x + w, y + h, z + t);
	glVertex3f(x + w, y, z + t);

	glVertex3f(x + w, y, z);
	glVertex3f(x + w, y + h, z);
	glVertex3f(x + w, y + h, z + t);

	glVertex3f(x, y, z);
	glVertex3f(x + w, y, z);
	glVertex3f(x + w, y, z + t);

	glVertex3f(x, y, z);
	glVertex3f(x + w, y, z + t);
	glVertex3f(x, y, z + t);

	glVertex3f(x, y + h, z);
	glVertex3f(x + w, y + h, z + t);
	glVertex3f(x + w, y + h, z);

	glVertex3f(x, y + h, z);
	glVertex3f(x, y + h, z + t);
	glVertex3f(x + w, y + h, z + t);

	glEnd();
}