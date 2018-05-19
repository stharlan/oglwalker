#pragma once

struct UserLocation {
	float azimuth;
	float elevation;
	float ex;
	float ez;
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
