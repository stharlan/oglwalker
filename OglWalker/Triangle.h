#pragma once

namespace oglw {

	class Triangle : I3DObject
	{
	public:
		Triangle();
		~Triangle();
		Triangle(const Triangle& t);
		Triangle& operator=(const Triangle& t);
		Triangle(glm::vec3& pp1, glm::vec3& pp2, glm::vec3& pp3);
		void Set(glm::vec3& pp1, glm::vec3& pp2, glm::vec3& pp3);
		void Draw();
		float MinX() { return fmin(fmin(p1.x, p1.y), p3.x); }
		float MaxX() { return fmax(fmax(p1.x, p2.x), p3.x); }
		float MinY() { return fmin(fmin(p1.y, p2.y), p3.y); }
		float MaxY() { return fmax(fmax(p1.y, p2.y), p3.y); }
		float MinZ() { return fmin(fmin(p1.z, p2.z), p3.z); }
		float MaxZ() { return fmax(fmax(p1.z, p2.z), p3.z); }
		glm::vec3 MinBox();
		glm::vec3 MaxBox();
		glm::vec3 SurfaceNormal();
		glm::vec3 Centroid();

		glm::vec3 p1;
		glm::vec3 p2;
		glm::vec3 p3;
		glm::vec3 sfcnrml;
		glm::vec3 cntrd;
	};

}