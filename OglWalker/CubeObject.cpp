#include "stdafx.h"
#include "CubeObject.h"


CubeObject::CubeObject()
{
	this->x = 10;
	this->y = 0;
	this->z = 10;
	this->dx = 10;
	this->dy = 10;
	this->dz = 10;
	this->BuildTris();
}

CubeObject::CubeObject(float px, float py, float pz, float pw, float ph, float pt)
{	
	this->x = px;
	this->y = py;
	this->z = pz;
	this->dx = pw;
	this->dy = ph;
	this->dz = pt;
	this->BuildTris();
}

CubeObject::~CubeObject()
{
}

void CubeObject::AddTri(
	float f1, float f2, float f3,
	float f4, float f5, float f6,
	float f7, float f8, float f9)
{
	Point p1(f1, f2, f3);
	Point p2(f4, f5, f6);
	Point p3(f7, f8, f9);
	Triangle t(p1, p2, p3);
	this->tris.push_back(t);
}


void CubeObject::BuildTris()
{
	this->tris.clear();
	AddTri(x, y, z, x + dx, y + dy, z, x + dx, y, z);
	AddTri(x, y, z,	x, y + dy, z, x + dx, y + dy, z);
	AddTri(x, y, z + dz, x + dx, y, z + dz, x + dx, y + dy, z + dz);
	AddTri(x, y, z + dz, x + dx, y + dy, z + dz, x, y + dy, z + dz);
	AddTri(x, y, z,	x, y, z + dz, x, y + dy, z + dz);
	AddTri(x, y, z,	x, y + dy, z + dz, x, y + dy, z);
	AddTri(x + dx, y, z, x + dx, y + dy, z + dz, x + dx, y, z + dz);
	AddTri(x + dx, y, z, x + dx, y + dy, z, x + dx, y + dy, z + dz);
	AddTri(x, y, z,	x + dx, y, z, x + dx, y, z + dz);
	AddTri(x, y, z,	x + dx, y, z + dz, x, y, z + dz);
	AddTri(x, y + dy, z, x + dx, y + dy, z + dz, x + dx, y + dy, z);
	AddTri(x, y + dy, z, x, y + dy, z + dz, x + dx, y + dy, z + dz);
}

void CubeObject::Draw()
{
	// y is the height
	glBegin(GL_TRIANGLES);

	for (std::vector<Triangle>::iterator it = this->tris.begin(); it != this->tris.end(); ++it)
		it->Draw();

	glEnd();
}
