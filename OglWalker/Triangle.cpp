#include "stdafx.h"
#include "Triangle.h"


Triangle::Triangle()
{
}

Triangle::~Triangle()
{
}

Triangle::Triangle(const Triangle& t)
{
	this->p1 = t.p1;
	this->p2 = t.p2;
	this->p3 = t.p3;
}

Triangle& Triangle::operator=(const Triangle& t)
{
	this->p1 = t.p1;
	this->p2 = t.p2;
	this->p3 = t.p3;
	return *this;
}

Triangle::Triangle(Point& pp1, Point& pp2, Point& pp3)
{
	this->Set(pp1, pp2, pp3);
}

void Triangle::Set(Point& pp1, Point& pp2, Point& pp3)
{
	this->p1 = pp1;
	this->p2 = pp2;
	this->p3 = pp3;
}

void Triangle::Draw()
{
	glVertex3f(this->p1.x, this->p1.y, this->p1.z);
	glVertex3f(this->p2.x, this->p2.y, this->p2.z);
	glVertex3f(this->p3.x, this->p3.y, this->p3.z);
}

Point& Triangle::MinBox()
{
	return Point(
		min(min(p1.x, p2.x), p3.x),
		min(min(p1.y, p2.y), p3.y),
		min(min(p1.z, p2.z), p3.z));
}

Point& Triangle::MaxBox()
{
	return Point(
		max(max(p1.x, p2.x), p3.x),
		max(max(p1.y, p2.y), p3.y),
		max(max(p1.z, p2.z), p3.z));
}
