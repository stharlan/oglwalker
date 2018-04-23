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

	float x;
	float y;
	float z;
};

