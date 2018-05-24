
#define USING_DIRECTX11
//#define USING_OPENGL
#define GLM_ENABLE_EXPERIMENTAL

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
#include <string>
#include <vector>
#include <fstream>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include "dddcommon.h"
#include "dx11i.h"
#include "opengli.h"

// define the screen resolution
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

LARGE_INTEGER pFreq;
LARGE_INTEGER pLast;

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

	std::ofstream dbg("debug_info.txt");
	char cdir[256];
	memset(cdir, 0, 256);
	GetCurrentDirectory(256, cdir);
	dbg << cdir << std::endl;
	dbg.flush();
	dbg.close();

	LARGE_INTEGER pThisTime;
	char TitleText[256];

	DDDCOMMON::TriangleMeshConfig m[3];
	memset(&m[0], 0, 2 * sizeof(DDDCOMMON::TriangleMeshConfig));
	DDDCOMMON::LoadTriangleMeshFromGLB("cat.glb", &m[0]);
	DDDCOMMON::CalculateBoundingBox(&m[0]);

	m[0].winding = DDDCOMMON::MeshConfigWinding::CounterClockwise;
	// SRT scale rotate translate (opposite multiply?)
	m[0].model = glm::mat4x4(1.0f)
		* glm::translate(glm::vec3(0.0f, -1.0f * m[0].bbox.ymin, -20.0f))
		* glm::scale(glm::vec3(1.0f, 1.0f, 1.0f))
		* glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//m[0].TextureFilename = std::string("redpix.png");
	m[0].TextureId = 0;
	
	glm::vec3 positions[] = {
		glm::vec3(-50, 0, -50),
		glm::vec3(50, 0, -50),
		glm::vec3(50, 0, 50),
		glm::vec3(-50, 0, 50)
	};
	glm::vec3 normals[] = {
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 0)
	};
	glm::vec2 texcoords[] = {
		glm::vec2(0.0f, 0.0f),
		glm::vec2(2.5f, 0.0f),
		glm::vec2(2.5f, 2.5f),
		glm::vec2(0.0f, 2.5f)
	};
	unsigned short indexes[] = { 0, 1, 3, 1, 2, 3 };
	m[1].indexes = indexes;
	m[1].model = glm::mat4x4(1.0f);
	m[1].normals = normals;
	m[1].NumIndexes = 6;
	m[1].NumNormals = 4;
	m[1].NumPositions = 4;
	m[1].positions = positions;
	m[1].NumTexCoords = 4;
	m[1].texcoords = texcoords;
	m[1].winding = DDDCOMMON::MeshConfigWinding::Clockwise;
	//m[1].TextureFilename = std::string("wp512.jpg"); // wp.png");
	m[1].TextureId = 1;

	const char* TextureList[] = { "redpix.png", "wp512.jpg" };
	std::vector<std::string> TextureVector(TextureList, std::end(TextureList));

	DDDCOMMON::CreateRoom(-25.0f, 0.0f, -25.0f, 50.0f, 8.0f, 50.0f, &m[2]);
	m[2].model = glm::mat4x4(1.0f);
	m[2].winding = DDDCOMMON::MeshConfigWinding::CounterClockwise;
	m[2].TextureId = 1;

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

	if (FALSE == DDDCOMMON::SetupDirectInput(hInst, hWnd)) return 0;

	// need to load all textures in one shot
	// and refer to them from the objects with id's
#ifdef USING_OPENGL
	if (FALSE == SHOGL::Init(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT)) return 0;
	if (FALSE == SHOGL::InitPipeline()) return 0;
	if (FALSE == SHOGL::InitGraphicsA(&m[0], 2)) return 0;
	if (FALSE == SHOGL::InitTextures(TextureVector)) return 0;
#endif
#ifdef USING_DIRECTX11
	if (FALSE == SHDX11::Init(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT)) return 0;
	if (FALSE == SHDX11::InitPipeline()) return 0;
	if (FALSE == SHDX11::InitGraphicsA(&m[0], 2)) return 0;
	if (FALSE == SHOGL::InitTextures(TextureVector)) return 0;
#endif

	DDDCOMMON::CleanupTriangleMeshConfig(&m[0]);
	//DDDCOMMON::CleanupTriangleMeshConfig(&m[1]);
	DDDCOMMON::CleanupTriangleMeshConfig(&m[2]);

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
			if (FALSE == SHOGL::UpdateFrame(hWnd)) break;
			if (FALSE == SHOGL::RenderFrame()) break;
#endif
#ifdef USING_DIRECTX11
			if (FALSE == SHDX11::UpdateFrame(hWnd))
				break;
			if (FALSE == SHDX11::RenderFrame())
				break;
#endif
		}
	}

	ShowCursor(TRUE);
	DDDCOMMON::CleanupDirectInput();
#ifdef USING_OPENGL
	SHOGL::Cleanup();
#endif
#ifdef USING_DIRECTX11
	SHDX11::Cleanup();
#endif

	return 0;
}
