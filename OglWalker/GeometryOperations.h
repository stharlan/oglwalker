#pragma once

bool RayIntersectsTriangle(glm::vec3& rayOrigin,
	glm::vec3& rayVector,
	oglw::Triangle& inTriangle,
	glm::vec3& outIntersectionPoint);
