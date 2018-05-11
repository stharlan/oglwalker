
#include "stdafx.h"

namespace glext {

	pfnglGenBuffers glGenBuffers;
	pfnglDeleteBuffers glDeleteBuffers;
	pfnglBindBuffer glBindBuffer;

	void InitializeExtensions()
	{
		glGenBuffers = (pfnglGenBuffers)wglGetProcAddress("glGenBuffers");
		glDeleteBuffers = (pfnglDeleteBuffers)wglGetProcAddress("glDeleteBuffers");
		glBindBuffer = (pfnglBindBuffer)wglGetProcAddress("glBindBuffer");
	}

}