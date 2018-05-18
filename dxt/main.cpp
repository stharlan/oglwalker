
#define GLM_ENABLE_EXPERIMENTAL

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <math.h>
#include <d3d11.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>

// define the screen resolution
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3d10.lib")

IDXGISwapChain *lpSwapchain = NULL;
ID3D11Device *lpDev = NULL;
ID3D11DeviceContext *lpDevcon = NULL;
ID3D11RenderTargetView *lpBackbuffer = NULL;
ID3D11VertexShader *lpVS = NULL;
ID3D11PixelShader *lpPS = NULL;
ID3D11Buffer *pVBuffer = NULL;
ID3D11Buffer* pWorldTransformBuffer = NULL;
ID3D11InputLayout *pLayout = NULL;

struct VERTEX { glm::vec3 pos; glm::vec4 clr; };    // a struct to define a vertex

struct REND_CONST_BUFFER
{
	glm::mat4x4 WorldMatrix;
};

BOOL InitD3D(HWND hWnd);
void CleanD3D(void);
BOOL RenderFrame(void);
BOOL UpdateFrame(void);
BOOL InitPipeline();
BOOL InitGraphics();

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

	if (FALSE == InitD3D(hWnd)) return 0;
	if (FALSE == InitPipeline()) return 0;
	if (FALSE == InitGraphics()) return 0;

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

			// Run game code here
			if (FALSE == UpdateFrame())
				break;
			if (FALSE == RenderFrame())
				break;

		}
	}

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
	sc.BufferDesc.RefreshRate.Numerator = 1;
	sc.BufferDesc.RefreshRate.Denominator = 60;
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

	// set the render target as the back buffer
	lpDevcon->OMSetRenderTargets(1, &lpBackbuffer, NULL);

	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;

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
	D3D11_INPUT_ELEMENT_DESC InputElementDescriptor[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	sCode = ReadTextFile("shaders.hlsl", &sSize);

	if (!sCode) return FALSE;

	//D3D10_SHADER_PACK_MATRIX_ROW_MAJOR

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
	if (FAILED(lpDev->CreateInputLayout(InputElementDescriptor, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout))) return FALSE;
	lpDevcon->IASetInputLayout(pLayout);

	return TRUE;
}

BOOL InitGraphics()
{
	D3D11_BUFFER_DESC VertexBufferDescriptor;
	D3D11_MAPPED_SUBRESOURCE ms;
	// create a triangle using the VERTEX struct
	VERTEX GeometryVertices[] =
	{
		//{ glm::vec3(0.0f, 0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) },
		//{ glm::vec3(0.45f, -0.5, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) },
		//{ glm::vec3(-0.45f, -0.5f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) }
		{ glm::vec3(-5.0f, -5.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ glm::vec3(-5.0f,  5.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ glm::vec3( 5.0f, -5.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ glm::vec3( 5.0f, -5.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ glm::vec3(-5.0f,  5.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ glm::vec3( 5.0f,  5.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) },

		{ glm::vec3(-10.0f, -5.0f,  10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ glm::vec3(-10.0f, -5.0f, -10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ glm::vec3( 10.0f, -5.0f,  10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ glm::vec3( 10.0f, -5.0f,  10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ glm::vec3(-10.0f, -5.0f, -10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ glm::vec3( 10.0f, -5.0f, -10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f) }
		
	};
	REND_CONST_BUFFER rcBuffer;
	D3D11_BUFFER_DESC rcBufferDesc;


	// create the vertex buffer
	ZeroMemory(&VertexBufferDescriptor, sizeof(VertexBufferDescriptor));

	VertexBufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	VertexBufferDescriptor.ByteWidth = sizeof(VERTEX) * _ARRAYSIZE(GeometryVertices);             // size is the VERTEX struct * 3
	VertexBufferDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	VertexBufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

	if (FAILED(lpDev->CreateBuffer(&VertexBufferDescriptor, NULL, &pVBuffer))) return FALSE;

	// copy the vertices into the buffer
	if (FAILED(lpDevcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) return false;
	memcpy(ms.pData, GeometryVertices, sizeof(GeometryVertices));                 // copy the data
	lpDevcon->Unmap(pVBuffer, NULL);                                      // unmap the buffer


																		  // create an identiy matrix
	rcBuffer.WorldMatrix = glm::mat4x4(1.0f);

	rcBufferDesc.ByteWidth = sizeof(REND_CONST_BUFFER);
	rcBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	rcBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	rcBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	rcBufferDesc.MiscFlags = 0;
	rcBufferDesc.StructureByteStride = 0;

	// Create the buffer.
	if (FAILED(lpDev->CreateBuffer(&rcBufferDesc, nullptr, &pWorldTransformBuffer))) return FALSE;

	// Set the buffer.
	if (FAILED(lpDevcon->Map(pWorldTransformBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) return false;
	memcpy(ms.pData, &rcBuffer, sizeof(REND_CONST_BUFFER));
	lpDevcon->Unmap(pWorldTransformBuffer, NULL);

	lpDevcon->VSSetConstantBuffers(0, 1, &pWorldTransformBuffer);

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
	REND_CONST_BUFFER rcBuffer;
	D3D11_MAPPED_SUBRESOURCE ms;
	static float var = 0.0f;
	var += 0.001f;
	glm::mat4x4 unTransposedWorldMatrix = glm::mat4x4(1.0f)
		* glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f)
		* glm::lookAt(
			glm::vec3(0.0f, 0.0f, 20.0f),
			glm::vec3(0.0f, 0.0f, 19.0f),
			glm::vec3(0.0f, 1.0f, 0.0))
		;
	rcBuffer.WorldMatrix = glm::transpose(unTransposedWorldMatrix);
	if (FAILED(lpDevcon->Map(pWorldTransformBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) return false;
	memcpy(ms.pData, &rcBuffer, sizeof(REND_CONST_BUFFER));
	lpDevcon->Unmap(pWorldTransformBuffer, NULL);
	return TRUE;
}

// this is the function used to render a single frame
BOOL RenderFrame(void)
{
	// clear the back buffer to a deep blue
	const float pColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	lpDevcon->ClearRenderTargetView(lpBackbuffer, pColor);

	// do 3D rendering on the back buffer here
	// select which vertex buffer to display
	UINT stride = sizeof(VERTEX);
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
	if (pLayout) pLayout->Release();
	if (pVBuffer) pVBuffer->Release();
	if (lpVS) lpVS->Release();
	if (lpPS) lpPS->Release();
	if (lpSwapchain) lpSwapchain->Release();
	if (lpBackbuffer) lpBackbuffer->Release();
	if (lpDev) lpDev->Release();
	if (lpDevcon) lpDevcon->Release();
}

