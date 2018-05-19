
#define GLM_ENABLE_EXPERIMENTAL
#define PI 3.14159f
#define DEG2RAD(x) (x * PI / 180.0f)

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dinput.h>
#include <math.h>
#include "dddcommon.h"

IDirectInput8 *pDirectInput8 = nullptr;
IDirectInputDevice8 *pKeyboard = nullptr;
IDirectInputDevice8 *pMouse = nullptr;

char* ReadTextFile(const char* filename, size_t* filesize)
{
	char* buffer = nullptr;
	FILE* f = nullptr;

	fopen_s(&f, filename, "r");
	fseek(f, 0, SEEK_END);
	*filesize = ftell(f);
	fseek(f, 0, SEEK_SET);
	buffer = (char*)malloc(*filesize);
	if (buffer) {
		memset(buffer, 0, *filesize);
		fread(buffer, *filesize, 1, f);
	}
	return buffer;
}

bool ReadMouseState(DIMOUSESTATE *pMouseState)
{
	HRESULT result;

	// Read the mouse device.
	result = pMouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)pMouseState);
	if (FAILED(result))
	{
		// If the mouse lost focus or was not acquired then try to get control back.
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			pMouse->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool ReadKeyboardState(unsigned char* keystate)
{
	HRESULT result;
	result = pKeyboard->GetDeviceState(sizeof(unsigned char[256]), (LPVOID)keystate);
	if (FAILED(result)) {
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
			pKeyboard->Acquire();
		}
		else {
			return false;
		}
	}
	return true;
}


void ProcessInput(UserLocation* pLoc, HWND hWnd)
{
	unsigned char keystate[256];
	DIMOUSESTATE mouseState;
	float WalkingStride = 0.001f;

	if (TRUE == ReadMouseState(&mouseState))
	{
		pLoc->azimuth -= (mouseState.lX / 8.0f);
		if (pLoc->azimuth < 0.0f) pLoc->azimuth = pLoc->azimuth + 360.0f;
		if (pLoc->azimuth > 360.0f) pLoc->azimuth = pLoc->azimuth - 360.0f;

		pLoc->elevation += (mouseState.lY / 8.0f);
		if (pLoc->elevation < -90.0f) pLoc->elevation = -90.0f;
		if (pLoc->elevation > 90.0f) pLoc->elevation = 90.0f;
	}
	if (TRUE == ReadKeyboardState(&keystate[0]))
	{
		if (keystate[DIK_ESCAPE] == (unsigned char)128) {
			DestroyWindow(hWnd);
		}

		float px = 0.0f, pz = 0.0f;
		float EyeAzimuthInRadians = 0.0f;
		float MovementDirectionInDegrees = 0.0f;

		// using standard wsad for movement
		// fwd bck strafe left and strafe right
		if (keystate[DIK_W] == (unsigned char)128) {
			if (keystate[DIK_A] == (unsigned char)128) {
				MovementDirectionInDegrees = pLoc->azimuth + 45.0f;
				EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
				px = sinf(EyeAzimuthInRadians);
				pz = cosf(EyeAzimuthInRadians);
			}
			else if (keystate[DIK_D] == (unsigned char)128) {
				MovementDirectionInDegrees = pLoc->azimuth - 45.0f;
				EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
				px = sinf(EyeAzimuthInRadians);
				pz = cosf(EyeAzimuthInRadians);
			}
			else {
				MovementDirectionInDegrees = pLoc->azimuth;
				EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
				px = sinf(EyeAzimuthInRadians);
				pz = cosf(EyeAzimuthInRadians);
			}
		}
		else if (keystate[DIK_S] == (unsigned char)128) {
			if (keystate[DIK_A] == (unsigned char)128) {
				MovementDirectionInDegrees = pLoc->azimuth + 135.0f;
				EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
				px = sinf(EyeAzimuthInRadians);
				pz = cosf(EyeAzimuthInRadians);
			}
			else if (keystate[DIK_D] == (unsigned char)128) {
				MovementDirectionInDegrees = pLoc->azimuth - 135.0f;
				EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
				px = sinf(EyeAzimuthInRadians);
				pz = cosf(EyeAzimuthInRadians);
			}
			else {
				MovementDirectionInDegrees = pLoc->azimuth + 180.0f;
				EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
				px = sinf(EyeAzimuthInRadians);
				pz = cosf(EyeAzimuthInRadians);
			}
		}
		else if (keystate[DIK_A] == (unsigned char)128) {
			MovementDirectionInDegrees = pLoc->azimuth + 90.0f;
			EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
			px = sinf(EyeAzimuthInRadians);
			pz = cosf(EyeAzimuthInRadians);
		}
		else if (keystate[DIK_D] == (unsigned char)128) {
			MovementDirectionInDegrees = pLoc->azimuth - 90.0f;
			EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
			px = sinf(EyeAzimuthInRadians);
			pz = cosf(EyeAzimuthInRadians);
		}

		//if (keystate[DIK_Z] == (unsigned char)128) ey -= WalkingStride;
		//if (keystate[DIK_C] == (unsigned char)128) ey += WalkingStride;

		pLoc->ex -= WalkingStride * px;
		pLoc->ez -= WalkingStride * pz;
	}
}

void CleanupDirectInput()
{
	if (pMouse != NULL) {
		pMouse->Unacquire();
		pMouse->Release();
		pMouse = NULL;
	}
	if (pKeyboard != NULL) {
		pKeyboard->Unacquire();
		pKeyboard->Release();
		pKeyboard = NULL;
	}
	if (pDirectInput8 != NULL) {
		pDirectInput8->Release();
		pDirectInput8 = NULL;
	}
}

BOOL SetupDirectInput(HINSTANCE hInst, HWND hWnd)
{
	if (FAILED(DirectInput8Create(hInst, 0x0800, IID_IDirectInput8, (void**)&pDirectInput8, NULL)))
		goto ERR;

	if (FAILED(pDirectInput8->CreateDevice(GUID_SysKeyboard, &pKeyboard, NULL)))
		goto ERR;

	if (FAILED(pKeyboard->SetDataFormat(&c_dfDIKeyboard)))
		goto ERR;

	if (FAILED(pKeyboard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
		goto ERR;

	if (FAILED(pKeyboard->Acquire()))
		goto ERR;

	if (FAILED(pDirectInput8->CreateDevice(GUID_SysMouse, &pMouse, NULL)))
		goto ERR;

	if (FAILED(pMouse->SetDataFormat(&c_dfDIMouse)))
		goto ERR;

	if (FAILED(pMouse->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
		goto ERR;

	if (FAILED(pMouse->Acquire()))
		goto ERR;

	return TRUE;

ERR:
	CleanupDirectInput();
	return FALSE;
}


