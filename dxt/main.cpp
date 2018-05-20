
//#define USING_DIRECTX11
#define USING_OPENGL

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3d10.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment(lib, "CORE_RL_Magick++_.lib")
#pragma comment(lib, "CORE_RL_MagickCore_.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew.lib")

#include <windows.h>
#include <stdio.h>
#include <fx/gltf.h>
#include "dddcommon.h"
#include "dx11i.h"
#include "opengli.h"

#include <glm/vec3.hpp>

// define the screen resolution
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

LARGE_INTEGER pFreq;
LARGE_INTEGER pLast;

void TestLoadGlb()
{
	std::ofstream dbg("c:\\temp\\glb_debug.txt");
	dbg << "opening file" << std::endl;
	fx::gltf::Document cloud = fx::gltf::LoadFromBinary("c:\\temp\\cloud.glb");

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
			free(usbuffer);

		}
	}

	dbg << "done" << std::endl;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, PWSTR pCmdLine, int nCmdShow)
{

	LARGE_INTEGER pThisTime;
	char TitleText[256];

	TestLoadGlb();
	if (true) return 0;

	WNDCLASSEX wcex = {};
	wcex.cbClsExtra = 0;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.cbWndExtra = 0;
	wcex.hCursor = (HCURSOR)LoadCursor(nullptr, IDC_ARROW);
	wcex.hIcon = nullptr;
	wcex.hIconSm = nullptr;
	wcex.hInstance = hInst;
	wcex.lpfnWndProc = WindowProc;
	wcex.lpszClassName = "DXTCLASS";
	wcex.lpszMenuName = nullptr;
	wcex.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClassEx(&wcex);

	RECT r = { 0,0,SCREEN_WIDTH,SCREEN_HEIGHT };
	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, TRUE);

	HWND hWnd = CreateWindow(
		"DXTCLASS",
		"DXT",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		r.right - r.left,
		r.bottom - r.top,
		nullptr,
		nullptr,
		hInst,
		nullptr);

	if (!hWnd) return 0;

	if (FALSE == SetupDirectInput(hInst, hWnd)) return 0;
#ifdef USING_OPENGL
	if (FALSE == SHOGL::Init(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT)) return 0;
	if (FALSE == SHOGL::InitPipeline()) return 0;
	if (FALSE == SHOGL::InitGraphics()) return 0;
	if (FALSE == SHOGL::InitTextures()) return 0;
#endif
#ifdef USING_DIRECTX11
	if (FALSE == SHDX11::Init(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT)) return 0;
	if (FALSE == SHDX11::InitPipeline()) return 0;
	if (FALSE == SHDX11::InitGraphics()) return 0;
	if (FALSE == SHDX11::InitTextures()) return 0;
#endif

	QueryPerformanceFrequency(&pFreq);
	QueryPerformanceCounter(&pLast);

	ShowCursor(FALSE);

	ShowWindow(hWnd, nCmdShow);

	// Enter the infinite message loop
	MSG msg = {};
	while (TRUE)
	{
		// Check to see if any messages are waiting in the queue
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// Translate the message and dispatch it to WindowProc()
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			// If the message is WM_QUIT, exit the while loop
			if (msg.message == WM_QUIT)
				break;
		}
		else {

			QueryPerformanceCounter(&pThisTime);
			LONGLONG fps = pFreq.QuadPart / (pThisTime.QuadPart - pLast.QuadPart);
			ZeroMemory(TitleText, 256);
			sprintf_s(TitleText, 256, "FPS %lli", fps);
			SetWindowText(hWnd, TitleText);
			pLast = pThisTime;

			// Run game code here
#ifdef USING_OPENGL
			if (FALSE == SHOGL::UpdateFrame(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT))
				break;
			if (FALSE == SHOGL::RenderFrame())
				break;
#endif
#ifdef USING_DIRECTX11
			if (FALSE == SHDX11::UpdateFrame(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT))
				break;
			if (FALSE == SHDX11::RenderFrame())
				break;
#endif
		}
	}

	ShowCursor(TRUE);
	CleanupDirectInput();
#ifdef USING_OPENGL
	SHOGL::Cleanup();
#endif
#ifdef USING_DIRECTX11
	SHDX11::Cleanup();
#endif
	return 0;
}
