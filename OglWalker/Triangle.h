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

	Point p1;
	Point p2;
	Point p3;
};

