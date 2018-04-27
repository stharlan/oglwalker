#pragma once
class Point
{
public:
	Point();
	~Point();
	Point(const Point& p);
	Point& operator=(const Point& p);
	Point(float px, float py, float pz);
	void Set(float px, float py, float pz);
	Point crossProduct(Point& v);
	float dotProduct(Point& v);
	Point operator+(Point& p);
	Point operator-(Point& p);
	Point operator*(float t);
	Point MakeUnit();
	float Length();
	float Distance(Point& p);

	float x;
	float y;
	float z;
};
