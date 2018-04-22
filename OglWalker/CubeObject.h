#pragma once
class CubeObject : I3DObject
{
public:
	CubeObject();
	CubeObject(float px, float py, float pz, float pw, float ph, float pt);
	~CubeObject();
	void Draw();
private:
	float x;
	float y;
	float z;
	float w;
	float h;
	float t;
};

