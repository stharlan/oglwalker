#pragma once

struct UserLocation {
	float azimuth;
	float elevation;
	float ex;
	float ez;
};

enum MeshConfigWinding {
	Clockwise,
	CounterClockwise
};

struct TriangleMeshConfig {
	glm::vec3 *positions = nullptr;
	UINT NumPositions = 0;
	glm::vec3 *normals = nullptr;
	UINT NumNormals = 0;
	USHORT *indexes = nullptr;
	UINT NumIndexes = 0;
	MeshConfigWinding winding;
	glm::mat4x4 model;
};

BOOL SetupDirectInput(HINSTANCE hInst, HWND hWnd);
void ProcessInput(UserLocation* pLoc, HWND hWnd);
void CleanupDirectInput();
char* ReadTextFile(const char* filename, size_t* filesize);

//BOOL SetupDirectInput(HINSTANCE hInst, HWND hWnd);
//void CleanupDirectInput();
//bool ReadKeyboardState(unsigned char* keystate);
//bool ReadMouseState(DIMOUSESTATE *pMouseState);
//void ProcessInput(UserLocation* pLoc);
