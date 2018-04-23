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

Point& Point::crossProduct(Point& v)
{
	return Point(
		(this->y * v.z) - (this->z * v.y),
		(this->x * v.z) - (this->z * v.x),
		(this->x * v.y) - (this->y * v.x)
	);
}

float Point::dotProduct(Point& v)
{
	return ((this->x * v.x) + (this->y * v.y) + (this->z * v.z));
}