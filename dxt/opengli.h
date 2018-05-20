#pragma once

namespace SHOGL {

	BOOL Init(HWND hWnd, unsigned int ScreenWidth, unsigned int ScreenHeight);
	BOOL InitPipeline(void);
	BOOL InitGraphicsA(DDDCOMMON::TriangleMeshConfig* configs, int NumConfigs);
	BOOL InitTextures(void);
	BOOL RenderFrame(void);
	BOOL UpdateFrame(HWND hWnd);
	void Cleanup(void);

}
