#pragma once

namespace DDDCOMMON {

	struct UserLocation {
		float azimuth;
		float elevation;
		float ex;
		float ey;
		float ez;
	};

	enum MeshConfigWinding {
		Clockwise,
		CounterClockwise
	};

	struct BoundingBox {
		float xmin;
		float xmax;
		float ymin;
		float ymax;
		float zmin;
		float zmax;
	};

	struct TriangleMeshConfig {
		glm::vec3 *positions = nullptr;
		UINT NumPositions = 0;
		glm::vec3 *normals = nullptr;
		UINT NumNormals = 0;
		glm::vec2 *texcoords = nullptr;
		UINT NumTexCoords = 0;
		USHORT *indexes = nullptr;
		UINT NumIndexes = 0;
		MeshConfigWinding winding;
		glm::mat4x4 model;
		BoundingBox bbox;
		std::string TextureFilename;
	};

	BOOL SetupDirectInput(HINSTANCE hInst, HWND hWnd);
	void ProcessInput(UserLocation* pLoc, HWND hWnd);
	void CleanupDirectInput();
	char* ReadTextFile(const char* filename, size_t* filesize);
	void LoadTriangleMeshFromGLB(const char* filename, TriangleMeshConfig *m);
	void CleanupTriangleMeshConfig(TriangleMeshConfig* c);
	void ReverseWinding(TriangleMeshConfig* config);
	void CalculateBoundingBox(TriangleMeshConfig* config);

	//BOOL SetupDirectInput(HINSTANCE hInst, HWND hWnd);
	//void CleanupDirectInput();
	//bool ReadKeyboardState(unsigned char* keystate);
	//bool ReadMouseState(DIMOUSESTATE *pMouseState);
	//void ProcessInput(UserLocation* pLoc);

}