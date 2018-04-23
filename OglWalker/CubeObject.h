#pragma once
class CubeObject : I3DObject
{
public:
	CubeObject();
	CubeObject(float px, float py, float pz, float pw, float ph, float pt);
	~CubeObject();
	void Draw();

private:
	void BuildTris();
	void AddTri(float f1, float f2, float f3,
		float f4, float f5, float f6,
		float f7, float f8, float f9);

private:
	float x;
	float y;
	float z;
	float w;
	float h;
	float t;
	std::vector<Triangle> tris;
};

