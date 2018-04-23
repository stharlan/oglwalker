#include "stdafx.h"
#include "Point.h"


Point::Point()
{
	this->Set(0.0f, 0.0f, 0.0f);
}


Point::~Point()
{
}

Point::Point(const Point& p)
{
	this->Set(p.x, p.y, p.z);
}

Point& Point::operator=(const Point& p)
{
	this->Set(p.x, p.y, p.z);
	return *this;
}

Point::Point(float px, float py, float pz)
{
	this->Set(px, py, pz);
}

void Point::Set(float px, float py, float pz)
{
	this->x = px;
	this->y = py;
	this->z = pz;
}