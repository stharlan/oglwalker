#pragma once

namespace SHDX11 {

	BOOL Init(HWND hWnd, unsigned int ScreenWidth, unsigned int ScreenHeight);
	BOOL InitPipeline(void);
	BOOL InitGraphics();
	BOOL InitGraphicsA(DDDCOMMON::TriangleMeshConfig* configs, int NumConfigs);
	BOOL RenderFrame(void);
	BOOL UpdateFrame(HWND hWnd);
	void Cleanup(void);

}