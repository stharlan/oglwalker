#pragma once

namespace SHDX11 {

	BOOL Init(HWND hWnd, unsigned int ScreenWidth, unsigned int ScreenHeight);
	BOOL InitPipeline(void);
	BOOL InitGraphics(void);
	BOOL InitTextures(void);
	BOOL RenderFrame(void);
	BOOL UpdateFrame(HWND hWnd, unsigned int ScreenWidth, unsigned int ScreenHeight);
	void Cleanup(void);

}