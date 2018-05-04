#include "stdafx.h"
#include "Triangle.h"

namespace oglw {

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
		//this->sfcnrml = this->SurfaceNormal().MakeUnit();
		this->sfcnrml = glm::normalize(this->SurfaceNormal());
		this->cntrd = this->Centroid();
	}

	Triangle& Triangle::operator=(const Triangle& t)
	{
		this->p1 = t.p1;
		this->p2 = t.p2;
		this->p3 = t.p3;
		//this->sfcnrml = this->SurfaceNormal().MakeUnit();
		this->sfcnrml = glm::normalize(this->SurfaceNormal());
		this->cntrd = this->Centroid();
		return *this;
	}

	Triangle::Triangle(glm::vec3& pp1, glm::vec3& pp2, glm::vec3& pp3)
	{
		this->Set(pp1, pp2, pp3);
	}

	void Triangle::Set(glm::vec3& pp1, glm::vec3& pp2, glm::vec3& pp3)
	{
		this->p1 = pp1;
		this->p2 = pp2;
		this->p3 = pp3;
		//this->sfcnrml = this->SurfaceNormal().MakeUnit();
		this->sfcnrml = glm::normalize(this->SurfaceNormal());
		this->cntrd = this->Centroid();
	}

	void Triangle::Draw()
	{
		glNormal3f(this->sfcnrml.x, this->sfcnrml.y, this->sfcnrml.z);
		glVertex3f(this->p1.x, this->p1.y, this->p1.z);
		glVertex3f(this->p2.x, this->p2.y, this->p2.z);
		glVertex3f(this->p3.x, this->p3.y, this->p3.z);
	}

	glm::vec3 Triangle::MinBox()
	{
		return glm::vec3(
			fmin(fmin(p1.x, p2.x), p3.x),
			fmin(fmin(p1.y, p2.y), p3.y),
			fmin(fmin(p1.z, p2.z), p3.z));
	}

	glm::vec3 Triangle::MaxBox()
	{
		return glm::vec3(
			fmax(fmax(p1.x, p2.x), p3.x),
			fmax(fmax(p1.y, p2.y), p3.y),
			fmax(fmax(p1.z, p2.z), p3.z));
	}

	glm::vec3 Triangle::SurfaceNormal()
	{
		glm::vec3 v1 = p2 - p1;
		glm::vec3 v2 = p3 - p1;
		//return v1.crossProduct(v2);
		return glm::cross(v1, v2);
	}

	glm::vec3 Triangle::Centroid()
	{
		return glm::vec3(
			(p1.x + p2.x + p3.x) / 3.0f,
			(p1.y + p2.y + p3.y) / 3.0f,
			(p1.z + p2.z + p3.z) / 3.0f);
	}

}