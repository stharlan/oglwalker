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
	Point& crossProduct(Point& v);
	float dotProduct(Point& v);

	float x;
	float y;
	float z;
};

inline Point& operator-(Point& p1, Point& p2)
{
	return Point(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z);
}

inline Point& operator+(Point& p1, Point& p2)
{
	return Point(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z);
}

inline Point& operator*(Point& p, float t)
{
	return Point(p.x * t, p.y * t, p.z * t);
}