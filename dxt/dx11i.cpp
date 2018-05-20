
#define GLM_ENABLE_EXPERIMENTAL
#define PI 3.14159f
#define DEG2RAD(x) (x * PI / 180.0f)


#include <Windows.h>
#include <d3d11.h>


#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <Magick++.h>
#include "dddcommon.h"
#include "dx11i.h"

namespace SHDX11 {

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

	BOOL Init(HWND hWnd, unsigned int ScreenWidth, unsigned int ScreenHeight)
	{
		DXGI_SWAP_CHAIN_DESC sc;
		ZeroMemory(&sc, sizeof(DXGI_SWAP_CHAIN_DESC));

		sc.BufferCount = 1;
		sc.BufferDesc.Height = ScreenHeight;
		sc.BufferDesc.Width = ScreenWidth;
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
		descDepth.Width = ScreenWidth;
		descDepth.Height = ScreenHeight;
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
		viewport.Width = (float)ScreenWidth;
		viewport.Height = (float)ScreenHeight;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		lpDevcon->RSSetViewports(1, &viewport);

		return TRUE;
	}

	BOOL InitPipeline(void)
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

		sCode = DDDCOMMON::ReadTextFile("shaders.hlsl", &sSize);

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

		free(sCode);

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
			{ glm::vec3(5.0f, -5.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(5.0f, -5.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(-5.0f,  5.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
			{ glm::vec3(5.0f,  5.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f) },

			{ glm::vec3(-10.0f, -5.0f,  10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f) },
			{ glm::vec3(-10.0f, -5.0f, -10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f) },
			{ glm::vec3(10.0f, -5.0f,  10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(10.0f, -5.0f,  10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
			{ glm::vec3(-10.0f, -5.0f, -10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f) },
			{ glm::vec3(10.0f, -5.0f, -10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f) }

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

	BOOL UpdateFrame(HWND hWnd, unsigned int ScreenWidth, unsigned int ScreenHeight)
	{
		VREND_CONST_BUFFER1 rcBuffer1;
		D3D11_MAPPED_SUBRESOURCE ms;
		static DDDCOMMON::UserLocation loc = { 0.0f, 0.0f, 0.0f, 20.0f };
		static float var = 0.0f;

		var += 0.01f;

		DDDCOMMON::ProcessInput(&loc, hWnd);
		glm::mat4x4 unTransposedWorldMatrix = glm::mat4x4(1.0f)
			* glm::perspective(glm::radians(45.0f), (float)ScreenWidth / (float)ScreenHeight, 0.1f, 100.0f)
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

	void Cleanup(void)
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

	//BOOL InitGraphicsA(TestMesh* m)
	//{
	//	return TRUE;
	//}
}

