
#include "stdafx.h"

namespace glext {

	pfnglGenBuffers glGenBuffers;
	pfnglDeleteBuffers glDeleteBuffers;

	void InitializeExtensions()
	{
		glGenBuffers = (pfnglGenBuffers)wglGetProcAddress("glGenBuffers");
		glDeleteBuffers = (pfnglDeleteBuffers)wglGetProcAddress("glDeleteBuffers");
	}

}