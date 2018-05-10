#pragma once

namespace glext {

	typedef void(*pfnglGenBuffers)(GLsizei, GLuint*);
	typedef void(*pfnglDeleteBuffers)(GLsizei, const GLuint*);

	extern pfnglGenBuffers glGenBuffers;
	extern pfnglDeleteBuffers glDeleteBuffers;

	void InitializeExtensions();

}