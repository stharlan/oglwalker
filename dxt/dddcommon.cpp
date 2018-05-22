
#define GLM_ENABLE_EXPERIMENTAL
#define PI 3.14159f
#define DEG2RAD(x) (x * PI / 180.0f)

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dinput.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <fx/gltf.h>
#include "dddcommon.h"

namespace DDDCOMMON {

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

	void LoadTriangleMeshFromGLB(const char* filename, TriangleMeshConfig *m)
	{
		std::ofstream dbg("c:\\temp\\glb_debug.txt");
		dbg << "opening file" << std::endl;
		fx::gltf::Document cloud = fx::gltf::LoadFromBinary(filename);

		dbg << "meshes " << cloud.meshes.size() << std::endl;
		for (std::vector<fx::gltf::Mesh>::iterator mi = cloud.meshes.begin(); mi != cloud.meshes.end(); ++mi) {
			// a mesh has primitives
			dbg << mi->name << std::endl;
			for (std::vector<fx::gltf::Primitive>::iterator pi = mi->primitives.begin(); pi != mi->primitives.end(); ++pi) {
				// a primitive has attributes
				fx::gltf::Attributes attrs = pi->attributes;
				dbg << "num attrs " << attrs.size() << std::endl;
				for (std::pair<std::string, uint32_t> element : attrs)
				{
					dbg << "=== ACCESSOR ===" << std::endl;
					dbg << "=== " << element.first << " = " << element.second << " ===" << std::endl;
					fx::gltf::Accessor ia = cloud.accessors.at(element.second);
					dbg << "acc index count " << ia.count << std::endl;
					dbg << "acc type " << (int)ia.type << std::endl;
					dbg << "acc comp type " << (int)ia.componentType << std::endl;
					dbg << "acc normalized " << ia.normalized << std::endl;
					// vec3 floats
					dbg << "acc buffer view " << ia.bufferView << std::endl;
					dbg << "acc byte offset " << ia.byteOffset << std::endl;
					dbg << "acc count " << ia.count << std::endl;

					fx::gltf::BufferView bv = cloud.bufferViews.at(ia.bufferView);
					dbg << "bv buffer " << bv.buffer << std::endl;
					dbg << "bv offset " << bv.byteOffset << std::endl;
					dbg << "bv byte length " << bv.byteLength << std::endl;
					dbg << "bv stride " << bv.byteStride << std::endl;

					fx::gltf::Buffer bfr = cloud.buffers.at(bv.buffer);
					glm::vec3* usbuffer = (glm::vec3*)malloc(bv.byteLength);
					memset(usbuffer, 0, bv.byteLength);
					memcpy(usbuffer, &bfr.data.at(bv.byteOffset), bv.byteLength);
					for (unsigned int x = 0; x < ia.count; x++) {
						dbg << usbuffer[x].r << ", " << usbuffer[x].g << ", " << usbuffer[x].b << std::endl;
					}

					if (element.first.compare("NORMAL") == 0) {
						m->NumNormals = ia.count;
						m->normals = usbuffer;
					}
					else if (element.first.compare("POSITION") == 0) {
						m->NumPositions = ia.count;
						m->positions = usbuffer;
					}

				}
				// a primitive has indices
				dbg << "=== INDICES ===" << std::endl;
				dbg << "indices index " << pi->indices << std::endl;
				fx::gltf::Accessor ia = cloud.accessors.at(pi->indices);
				dbg << "=== ACCESSOR ===" << std::endl;
				dbg << "acc index count " << ia.count << std::endl;
				//None, 0
				//Scalar, 1
				//Vec2, 2
				//Vec3, 3
				//Vec4, 4
				//Mat2, 5
				//Mat3, 6
				//Mat4 7
				// scalar type 1
				dbg << "acc type " << (int)ia.type << std::endl;
				//None = 0,
				//Byte = 5120,
				//UnsignedByte = 5121,
				//Short = 5122,
				//UnsignedShort = 5123,
				//UnsignedInt = 5125,
				//Float = 5126
				// this is an unsigned short
				dbg << "acc comp type " << (int)ia.componentType << std::endl;
				dbg << "acc normalized " << ia.normalized << std::endl;
				dbg << "acc buffer view " << ia.bufferView << std::endl;
				dbg << "acc byte offset " << ia.byteOffset << std::endl;
				dbg << "acc count " << ia.count << std::endl;
				fx::gltf::BufferView bv = cloud.bufferViews.at(ia.bufferView);
				dbg << "bv buffer " << bv.buffer << std::endl;
				dbg << "bv offset " << bv.byteOffset << std::endl;
				dbg << "bv byte length " << bv.byteLength << std::endl;
				dbg << "bv stride " << bv.byteStride << std::endl;

				fx::gltf::Buffer bfr = cloud.buffers.at(bv.buffer);
				unsigned short * usbuffer = (unsigned short*)malloc(bv.byteLength);
				memset(usbuffer, 0, bv.byteLength);
				memcpy(usbuffer, &bfr.data.at(bv.byteOffset), bv.byteLength);
				for (unsigned int x = 0; x < ia.count; x++) {
					dbg << usbuffer[x] << std::endl;
				}

				m->NumIndexes = ia.count;
				m->indexes = usbuffer;

			}
		}

		dbg << "done" << std::endl;
	}

	void CleanupTriangleMeshConfig(TriangleMeshConfig* c)
	{
		if(c->normals) free(c->normals);
		if(c->positions) free(c->positions);
		if(c->indexes) free(c->indexes);
	}

	void ReverseWinding(TriangleMeshConfig* config)
	{
		USHORT temp = 0;
		for (int i = 0; i < config->NumIndexes; i+=3) {
			temp = config->indexes[i];
			config->indexes[i] = config->indexes[i + 2];
			config->indexes[i + 2] = temp;
		}
	}

	void CalculateBoundingBox(TriangleMeshConfig* config) {
		config->bbox.xmin = FLT_MAX;
		config->bbox.xmax = FLT_MIN;
		config->bbox.ymin = FLT_MAX;
		config->bbox.ymax = FLT_MIN;
		config->bbox.zmin = FLT_MAX;
		config->bbox.zmax = FLT_MIN;
		for (int i = 0; i < config->NumPositions; i++) {
			if (config->positions[i].x > config->bbox.xmax) config->bbox.xmax = config->positions[i].x;
			if (config->positions[i].x < config->bbox.xmin) config->bbox.xmin = config->positions[i].x;
			if (config->positions[i].y > config->bbox.ymax) config->bbox.ymax = config->positions[i].y;
			if (config->positions[i].y < config->bbox.ymin) config->bbox.ymin = config->positions[i].y;
			if (config->positions[i].z > config->bbox.zmax) config->bbox.zmax = config->positions[i].z;
			if (config->positions[i].z < config->bbox.zmin) config->bbox.zmin = config->positions[i].z;
		}
	}
}

