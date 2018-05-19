
#define GLM_ENABLE_EXPERIMENTAL
#define PI 3.14159f
#define DEG2RAD(x) (x * PI / 180.0f)

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <math.h>
#include <d3d11.h>
#include <dinput.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <Magick++.h>
#include "WICTextureLoader.h"

// define the screen resolution
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 1024

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3d10.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment(lib, "CORE_RL_Magick++_.lib")
#pragma comment(lib, "CORE_RL_MagickCore_.lib")

HWND g_hWnd = NULL;
LARGE_INTEGER pFreq;
LARGE_INTEGER pLast;

IDXGISwapChain *lpSwapchain = nullptr;
ID3D11Device *lpDev = nullptr;
ID3D11DeviceContext *lpDevcon = nullptr;
ID3D11RenderTargetView *lpBackbuffer = nullptr;
ID3D11VertexShader *lpVS = nullptr;
ID3D11PixelShader *lpPS = nullptr;
ID3D11Buffer *pVBuffer = nullptr;
ID3D11Buffer *pWorldTransformBuffer = nullptr;
ID3D11Buffer *pLightInfoBuffer = nullptr;
ID3D11InputLayout *pLayout = nullptr;
ID3D11Texture2D* pDepthStencil = nullptr;
ID3D11DepthStencilView* pDSV = nullptr;
ID3D11DepthStencilState *pDSState = nullptr;
ID3D11Texture2D *pTextureResource = nullptr;
ID3D11ShaderResourceView *pTextureResView = nullptr;
ID3D11SamplerState *pSamplerState = nullptr;

IDirectInput8 *pDirectInput8 = nullptr;
IDirectInputDevice8 *pKeyboard = nullptr;
IDirectInputDevice8 *pMouse = nullptr;

struct VERTEX2 { glm::vec3 pos; glm::vec4 clr; glm::vec3 nrml; glm::vec2 texc; };    // a struct to define a vertex

struct VREND_CONST_BUFFER1
{
	glm::mat4x4 WorldMatrix;
};

struct PREND_CONST_BUFFER
{
	glm::vec4 f4LightDiffuse;
	glm::vec4 f4LightAmbient;
	glm::vec3 f3LightDir;
	float f1;
};

BOOL InitD3D(HWND hWnd);
void CleanD3D(void);
BOOL RenderFrame(void);
BOOL UpdateFrame(void);
BOOL InitPipeline();
BOOL InitGraphics();
BOOL InitTextures();

struct UserLocation {
	float azimuth;
	float elevation;
	float ex;
	float ez;
};

BOOL SetupDirectInput(HINSTANCE hInst, HWND hWnd);
void CleanupDirectInput();
bool ReadKeyboardState(unsigned char* keystate);
bool ReadMouseState(DIMOUSESTATE *pMouseState);
void ProcessInput(UserLocation* pLoc);

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
	if (FALSE == InitD3D(hWnd)) return 0;
	if (FALSE == InitPipeline()) return 0;
	if (FALSE == InitGraphics()) return 0;
	if (FALSE == InitTextures()) return 0;

	QueryPerformanceFrequency(&pFreq);
	QueryPerformanceCounter(&pLast);

	ShowCursor(FALSE);

	g_hWnd = hWnd;

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
			if (FALSE == UpdateFrame())
				break;
			if (FALSE == RenderFrame())
				break;

		}
	}

	ShowCursor(TRUE);
	CleanupDirectInput();
	CleanD3D();

	return 0;
}

BOOL InitD3D(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sc;
	ZeroMemory(&sc, sizeof(DXGI_SWAP_CHAIN_DESC));

	sc.BufferCount = 1;
	sc.BufferDesc.Height = SCREEN_HEIGHT;
	sc.BufferDesc.Width = SCREEN_WIDTH;
	sc.BufferDesc.RefreshRate.Numerator = 60;
	sc.BufferDesc.RefreshRate.Denominator = 1;
	sc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	sc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sc.OutputWindow = hWnd;
	sc.SampleDesc.Count = 1;
	sc.SampleDesc.Quality = 0;
	sc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sc.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};

	D3D_FEATURE_LEVEL chosenLevel;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_SINGLETHREADED,
		featureLevels,
		_ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&sc,
		&lpSwapchain,
		&lpDev,
		&chosenLevel,
		&lpDevcon);

	if (FAILED(hr)) return FALSE;

	// get the address of the back buffer
	ID3D11Texture2D *lpTexture2D;
	if (FAILED(lpSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&lpTexture2D))) return FALSE;

	// use the back buffer address to create the render target
	if (FAILED(lpDev->CreateRenderTargetView(lpTexture2D, NULL, &lpBackbuffer))) return FALSE;

	// ===============
	// depth stencil =
	// ===============
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(D3D11_TEXTURE2D_DESC));
	descDepth.Width = SCREEN_WIDTH;
	descDepth.Height = SCREEN_HEIGHT;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	if (FAILED(lpDev->CreateTexture2D(&descDepth, NULL, &pDepthStencil))) return false;

	D3D11_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Stencil test parameters
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state
	if (FAILED(lpDev->CreateDepthStencilState(&dsDesc, &pDSState))) return false;

	lpDevcon->OMSetDepthStencilState(pDSState, 1);

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	// Create the depth stencil view
	hr = lpDev->CreateDepthStencilView(pDepthStencil, &descDSV, &pDSV);
	if (FAILED(hr)) {
		unsigned int blah = hr;
		return false;
	}

	// set the render target as the back buffer
	//lpDevcon->OMSetRenderTargets(1, &lpBackbuffer, NULL);
	lpDevcon->OMSetRenderTargets(1, &lpBackbuffer, pDSV);

	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)SCREEN_WIDTH;
	viewport.Height = (float)SCREEN_HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	lpDevcon->RSSetViewports(1, &viewport);

	return TRUE;
}

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
		ZeroMemory(buffer, *filesize);
		fread(buffer, *filesize, 1, f);
	}
	return buffer;
}

BOOL InitPipeline()
{
	// load and compile the two shaders
	ID3D10Blob* VS = nullptr;
	ID3D10Blob* PS = nullptr;
	ID3D10Blob* pErrs = nullptr;
	char *sCode = nullptr;
	size_t sSize = 0;
	D3D11_INPUT_ELEMENT_DESC InputElementDescriptor2[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	sCode = ReadTextFile("shaders.hlsl", &sSize);

	if (!sCode) return FALSE;

	if (FAILED(D3D10CompileShader(sCode, sSize, nullptr, nullptr, nullptr, 
		"VShader", "vs_4_0", 0, &VS, &pErrs)))
	{
		free(sCode);
		return FALSE;
	}

	if (FAILED(D3D10CompileShader(sCode, sSize, nullptr, nullptr, nullptr, 
		"PShader", "ps_4_0", 0, &PS, &pErrs)))
	{
		free(sCode);
		return FALSE;
	}

	// encapsulate both shaders into shader objects
	if (FAILED(lpDev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &lpVS))) return FALSE;
	if (FAILED(lpDev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &lpPS))) return FALSE;

	// set the shader objects
	lpDevcon->VSSetShader(lpVS, 0, 0);
	lpDevcon->PSSetShader(lpPS, 0, 0);

	// create the input layout object
	if (FAILED(lpDev->CreateInputLayout(InputElementDescriptor2, _ARRAYSIZE(InputElementDescriptor2), 
		VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout))) return FALSE;
	lpDevcon->IASetInputLayout(pLayout);

	return TRUE;
}

BOOL InitGraphics()
{
	D3D11_BUFFER_DESC VertexBufferDescriptor;
	D3D11_MAPPED_SUBRESOURCE ms;
	// create a triangle using the VERTEX struct
	// windind is CW
	VERTEX2 GeometryVertices[] =
	{
		//{ glm::vec3(0.0f, 0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) },
		//{ glm::vec3(0.45f, -0.5, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) },
		//{ glm::vec3(-0.45f, -0.5f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) }
		{ glm::vec3(-5.0f, -5.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
		{ glm::vec3(-5.0f,  5.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
		{ glm::vec3( 5.0f, -5.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f) },
		{ glm::vec3( 5.0f, -5.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f) },
		{ glm::vec3(-5.0f,  5.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
		{ glm::vec3( 5.0f,  5.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f) },

		{ glm::vec3(-10.0f, -5.0f,  10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f) },
		{ glm::vec3(-10.0f, -5.0f, -10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f) },
		{ glm::vec3( 10.0f, -5.0f,  10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
		{ glm::vec3( 10.0f, -5.0f,  10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
		{ glm::vec3(-10.0f, -5.0f, -10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f) },
		{ glm::vec3( 10.0f, -5.0f, -10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f) }
		
	};
	VREND_CONST_BUFFER1 rcBuffer1;
	D3D11_BUFFER_DESC rcBufferDesc;
	PREND_CONST_BUFFER PixelRendererConstBuffer;
	D3D11_BUFFER_DESC PixelRendererConstBufferDesc;

	// create the vertex buffer
	ZeroMemory(&VertexBufferDescriptor, sizeof(VertexBufferDescriptor));

	VertexBufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	VertexBufferDescriptor.ByteWidth = sizeof(VERTEX2) * _ARRAYSIZE(GeometryVertices);             // size is the VERTEX struct * 3
	VertexBufferDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	VertexBufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

	if (FAILED(lpDev->CreateBuffer(&VertexBufferDescriptor, NULL, &pVBuffer))) return FALSE;

	// copy the vertices into the buffer
	if (FAILED(lpDevcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) return false;
	memcpy(ms.pData, GeometryVertices, sizeof(GeometryVertices));                 // copy the data
	lpDevcon->Unmap(pVBuffer, NULL);                                      // unmap the buffer


																		  // create an identiy matrix
	rcBuffer1.WorldMatrix = glm::mat4x4(1.0f);

	rcBufferDesc.ByteWidth = sizeof(VREND_CONST_BUFFER1);
	rcBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	rcBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	rcBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	rcBufferDesc.MiscFlags = 0;
	rcBufferDesc.StructureByteStride = 0;

	// Create the buffer.
	if (FAILED(lpDev->CreateBuffer(&rcBufferDesc, nullptr, &pWorldTransformBuffer))) return FALSE;

	// Set the buffer.
	if (FAILED(lpDevcon->Map(pWorldTransformBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) return false;
	memcpy(ms.pData, &rcBuffer1, sizeof(VREND_CONST_BUFFER1));
	lpDevcon->Unmap(pWorldTransformBuffer, NULL);

	lpDevcon->VSSetConstantBuffers(0, 1, &pWorldTransformBuffer);

	// pixel shader const buffer
	//PixelRendererConstBuffer.f3LightDir = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
	PixelRendererConstBuffer.f3LightDir = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
	PixelRendererConstBuffer.f4LightAmbient = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	PixelRendererConstBuffer.f4LightDiffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	PixelRendererConstBuffer.f1 = 0.0f;

	PixelRendererConstBufferDesc.ByteWidth = sizeof(PREND_CONST_BUFFER);
	PixelRendererConstBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	PixelRendererConstBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	PixelRendererConstBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	PixelRendererConstBufferDesc.MiscFlags = 0;
	PixelRendererConstBufferDesc.StructureByteStride = 0;

	// Create the buffer.
	if (FAILED(lpDev->CreateBuffer(&PixelRendererConstBufferDesc, nullptr, &pLightInfoBuffer))) return false;

	// Set the buffer.
	if (FAILED(lpDevcon->Map(pLightInfoBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) return false;
	memcpy(ms.pData, &PixelRendererConstBuffer, sizeof(PREND_CONST_BUFFER));
	lpDevcon->Unmap(pLightInfoBuffer, NULL);

	lpDevcon->PSSetConstantBuffers(1, 1, &pLightInfoBuffer);

	return TRUE;
}

BOOL InitTextures()
{
	size_t maxsize;
	Magick::Image m_image;
	Magick::Blob m_blob;

	try {
		m_image.read("c:\\temp\\me.jpg");
		m_image.write(&m_blob, "RGBA");
	}
	catch (Magick::Error& Error) {
		return FALSE;
	}

	//unsigned int imageWidth = m_image.columns();
	//unsigned int imageHeight = m_image.rows();
	//const void* imageData = m_blob.data();
	//unsigned int imageWidth = 2;
	//unsigned int imageHeight = 2;
	//char imageData [] = {
		//0, 255, 0, 255,
		//0, 255, 0, 255,
		//0, 255, 0, 255,
		//0, 255, 0, 255
	//};

	switch (lpDev->GetFeatureLevel())
	{
	case D3D_FEATURE_LEVEL_9_1:
	case D3D_FEATURE_LEVEL_9_2:
		maxsize = 2048 /*D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
		break;

	case D3D_FEATURE_LEVEL_9_3:
		maxsize = 4096 /*D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
		break;

	case D3D_FEATURE_LEVEL_10_0:
	case D3D_FEATURE_LEVEL_10_1:
		maxsize = 8192 /*D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
		break;

	default:
		maxsize = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
		break;
	}

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = m_image.columns();
	desc.Height = m_image.rows();
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	if (FAILED(lpDev->CreateTexture2D(&desc, nullptr, &pTextureResource))) return FALSE;

	// create a texture resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	if (FAILED(lpDev->CreateShaderResourceView(pTextureResource, &SRVDesc, &pTextureResView))) return FALSE;

	lpDevcon->PSSetShaderResources(0, 1, &pTextureResView);

	// set the texture data
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	if (FAILED(lpDevcon->Map(pTextureResource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) return FALSE;
	memcpy(mappedResource.pData, m_blob.data(), m_blob.length());
	lpDevcon->Unmap(pTextureResource, 0);

	// create a sampler
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	if (FAILED(lpDev->CreateSamplerState(&samplerDesc, &pSamplerState))) return FALSE;

	lpDevcon->PSSetSamplers(0, 1, &pSamplerState);

	return TRUE;
}

glm::mat4 camera(float Translate, glm::vec2 const & Rotate)
{
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.f);
	glm::mat4 View = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Translate));
	View = glm::rotate(View, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
	View = glm::rotate(View, Rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
	return Projection * View * Model;
}

BOOL UpdateFrame(void)
{
	VREND_CONST_BUFFER1 rcBuffer1;
	D3D11_MAPPED_SUBRESOURCE ms;
	static UserLocation loc = { 0.0f, 0.0f, 0.0f, 20.0f };
	static float var = 0.0f;

	var += 0.01f;

	ProcessInput(&loc);
	glm::mat4x4 unTransposedWorldMatrix = glm::mat4x4(1.0f)
		* glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f)
		* glm::lookAt(
			glm::vec3(loc.ex, 0.0f, loc.ez),
			glm::vec3(loc.ex - sinf(DEG2RAD(loc.azimuth)), 0.0f - sinf(DEG2RAD(loc.elevation)), loc.ez - cosf(DEG2RAD(loc.azimuth))),
			glm::vec3(0.0f, 1.0f, 0.0))
		* glm::rotate(glm::radians(var), glm::vec3(0.0f, 1.0f, 0.0f))
		;
	rcBuffer1.WorldMatrix = glm::transpose(unTransposedWorldMatrix);
	if (FAILED(lpDevcon->Map(pWorldTransformBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) return false;
	memcpy(ms.pData, &rcBuffer1, sizeof(VREND_CONST_BUFFER1));
	lpDevcon->Unmap(pWorldTransformBuffer, NULL);
	return TRUE;
}

// this is the function used to render a single frame
BOOL RenderFrame(void)
{
	// clear the back buffer to a deep blue
	const float pColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	lpDevcon->ClearRenderTargetView(lpBackbuffer, pColor);
	lpDevcon->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// do 3D rendering on the back buffer here
	// select which vertex buffer to display
	UINT stride = sizeof(VERTEX2);
	UINT offset = 0;
	lpDevcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);

	// select which primtive type we are using
	lpDevcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// draw the vertex buffer to the back buffer
	lpDevcon->Draw(12, 0);

	// switch the back buffer and the front buffer
	if (FAILED(lpSwapchain->Present(0, 0))) return FALSE;

	return TRUE;
}

void CleanD3D(void)
{
	if (lpSwapchain) lpSwapchain->SetFullscreenState(FALSE, NULL);
	if (pSamplerState) pSamplerState->Release();
	if (pTextureResource) pTextureResource->Release();
	if (pTextureResView) pTextureResView->Release();
	if (pDSV) pDSV->Release();
	if (pDSState) pDSState->Release();
	if (pDepthStencil) pDepthStencil->Release();
	if (pLayout) pLayout->Release();
	if (pVBuffer) pVBuffer->Release();
	if (pWorldTransformBuffer) pWorldTransformBuffer->Release();
	if (pLightInfoBuffer) pLightInfoBuffer->Release();
	if (lpVS) lpVS->Release();
	if (lpPS) lpPS->Release();
	if (lpSwapchain) lpSwapchain->Release();
	if (lpBackbuffer) lpBackbuffer->Release();
	if (lpDev) lpDev->Release();
	if (lpDevcon) lpDevcon->Release();
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

void ProcessInput(UserLocation* pLoc)
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
			DestroyWindow(g_hWnd);
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
