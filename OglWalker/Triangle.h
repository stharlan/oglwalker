#pragma once
class Triangle : I3DObject
{
public:
	Triangle();
	~Triangle();
	Triangle(const Triangle& t);
	Triangle& operator=(const Triangle& t);
	Triangle(Point& pp1, Point& pp2, Point& pp3);
	void Set(Point& pp1, Point& pp2, Point& pp3);
	void Draw();
	float MinX() { return min(min(p1.x, p2.x), p3.x); }
	float MaxX() { return max(max(p1.x, p2.x), p3.x); }
	float MinY() { return min(min(p1.y, p2.y), p3.y); }
	float MaxY() { return max(max(p1.y, p2.y), p3.y); }
	float MinZ() { return min(min(p1.z, p2.z), p3.z); }
	float MaxZ() { return max(max(p1.z, p2.z), p3.z); }
	Point& MinBox();
	Point& MaxBox();

	Point p1;
	Point p2;
	Point p3;
};

