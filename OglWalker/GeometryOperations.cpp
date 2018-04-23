
#include "stdafx.h"

bool RayIntersectsTriangle(Point& rayOrigin,
	Point& rayVector,
	Triangle& inTriangle,
	Point& outIntersectionPoint)
{
	const float EPSILON = 0.0000001f;
	Point vertex0 = inTriangle.p1;
	Point vertex1 = inTriangle.p2;
	Point vertex2 = inTriangle.p3;
	Point edge1, edge2, h, s, q;
	float a, f, u, v;
	edge1 = vertex1 - vertex0;
	edge2 = vertex2 - vertex0;
	h = rayVector.crossProduct(edge2);
	a = edge1.dotProduct(h);
	if (a > -EPSILON && a < EPSILON)
		return false;
	f = 1 / a;
	s = rayOrigin - vertex0;
	u = f * (s.dotProduct(h));
	if (u < 0.0 || u > 1.0)
		return false;
	q = s.crossProduct(edge1);
	v = f * rayVector.dotProduct(q);
	if (v < 0.0 || u + v > 1.0)
		return false;
	// At this stage we can compute t to find out where the intersection point is on the line.
	float t = f * edge2.dotProduct(q);
	if (t > EPSILON) // ray intersection
	{
		outIntersectionPoint = rayOrigin + rayVector * t;
		return true;
	}
	else // This means that there is a line intersection but not a ray intersection.
		return false;
}